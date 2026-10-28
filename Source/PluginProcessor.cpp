#include "PluginProcessor.h"
#include "PluginEditor.h"

TitanTuneMetalAudioProcessor::TitanTuneMetalAudioProcessor()
    : AudioProcessor (BusesProperties()
        .withInput ("Input", juce::AudioChannelSet::stereo(), true)
        .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      parameters (*this, nullptr, "PARAMETERS", createParameterLayout())
{
}

juce::AudioProcessorValueTreeState::ParameterLayout TitanTuneMetalAudioProcessor::createParameterLayout()
{
    using APF = juce::AudioParameterFloat;
    using APB = juce::AudioParameterBool;
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    layout.add (std::make_unique<APF> ("transpose", "Transpose",
        juce::NormalisableRange<float> (-36.0f, 12.0f, 1.0f), 0.0f,
        juce::AudioParameterFloatAttributes().withLabel ("st")));
    layout.add (std::make_unique<APF> ("blend", "Blend",
        juce::NormalisableRange<float> (0.0f, 1.0f, 0.01f), 1.0f));
    layout.add (std::make_unique<APF> ("focus", "Low-End Focus",
        juce::NormalisableRange<float> (0.0f, 1.0f, 0.01f), 0.25f));
    layout.add (std::make_unique<APF> ("gate", "Gate Threshold",
        juce::NormalisableRange<float> (-90.0f, -18.0f, 1.0f), -65.0f,
        juce::AudioParameterFloatAttributes().withLabel ("dB")));
    layout.add (std::make_unique<APB> ("gateOn", "Gate On", true));
    return layout;
}

void TitanTuneMetalAudioProcessor::PhaseVocoderPitchShifter::prepare (double sr, int maxBlockSize)
{
    sampleRate = sr;
    scratchSize = juce::jmax (1024, maxBlockSize);
    inputScratch.setSize (channels, scratchSize, false, true, true);
    outputScratch.setSize (channels, scratchSize, false, true, true);
    // Signalsmith Stretch is a polyphonic phase-vocoder-style pitch/time engine.
    // It replaces the previous dual-delay-head sweep, the source of audible repeats.
    stretcher.presetDefault (channels, static_cast<float> (sampleRate));
    stretcher.reset();
    currentSemitones = 0.0f;
    latencySamples = stretcher.inputLatency() + stretcher.outputLatency();
}

void TitanTuneMetalAudioProcessor::PhaseVocoderPitchShifter::reset()
{
    stretcher.reset();
    inputScratch.clear();
    outputScratch.clear();
}

void TitanTuneMetalAudioProcessor::PhaseVocoderPitchShifter::setSemitones (float st)
{
    if (std::abs (st - currentSemitones) > 0.001f)
    {
        currentSemitones = st;
        // A small tonality limit can retain some upper-harmonic character on guitar.
        stretcher.setTransposeSemitones (st, static_cast<float> (8000.0 / sampleRate));
    }
}

void TitanTuneMetalAudioProcessor::PhaseVocoderPitchShifter::process (
    const juce::AudioBuffer<float>& input, juce::AudioBuffer<float>& output,
    int startSample, int numSamples)
{
    if (std::abs (currentSemitones) < 0.001f)
    {
        for (int ch = 0; ch < juce::jmin (output.getNumChannels(), input.getNumChannels()); ++ch)
            output.copyFrom (ch, startSample, input, ch, startSample, numSamples);
        return;
    }

    int processed = 0;
    while (processed < numSamples)
    {
        const int count = juce::jmin (scratchSize, numSamples - processed);
        for (int ch = 0; ch < channels; ++ch)
        {
            auto* in = inputScratch.getWritePointer (ch);
            const int sourceChannel = juce::jmin (ch, input.getNumChannels() - 1);
            const auto* source = input.getReadPointer (sourceChannel, startSample + processed);
            juce::FloatVectorOperations::copy (in, source, count);
        }

        float* inputPointers[channels] = { inputScratch.getWritePointer (0), inputScratch.getWritePointer (1) };
        float* outputPointers[channels] = { outputScratch.getWritePointer (0), outputScratch.getWritePointer (1) };
        stretcher.process (inputPointers, count, outputPointers, count);

        for (int ch = 0; ch < juce::jmin (output.getNumChannels(), channels); ++ch)
            output.copyFrom (ch, startSample + processed, outputScratch, ch, 0, count);
        processed += count;
    }
}

void TitanTuneMetalAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    pitchShifter.prepare (sampleRate, samplesPerBlock);
    shiftedBuffer.setSize (2, juce::jmax (16384, samplesPerBlock), false, true, true);
    setLatencySamples (pitchShifter.getLatencySamples());
    highPassL.setType (juce::dsp::StateVariableTPTFilterType::highpass);
    highPassR.setType (juce::dsp::StateVariableTPTFilterType::highpass);
    lowPassL.setType (juce::dsp::StateVariableTPTFilterType::lowpass);
    lowPassR.setType (juce::dsp::StateVariableTPTFilterType::lowpass);
    highPassL.prepare ({ sampleRate, (juce::uint32) samplesPerBlock, 1 });
    highPassR.prepare ({ sampleRate, (juce::uint32) samplesPerBlock, 1 });
    lowPassL.prepare ({ sampleRate, (juce::uint32) samplesPerBlock, 1 });
    lowPassR.prepare ({ sampleRate, (juce::uint32) samplesPerBlock, 1 });
    highPassL.setCutoffFrequency (28.0f);
    highPassR.setCutoffFrequency (28.0f);
    lowPassL.setCutoffFrequency (18000.0f);
    lowPassR.setCutoffFrequency (18000.0f);
    gateEnvelopeL = gateEnvelopeR = 0.0f;
}

void TitanTuneMetalAudioProcessor::releaseResources() {}

bool TitanTuneMetalAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto& mainIn = layouts.getMainInputChannelSet();
    const auto& mainOut = layouts.getMainOutputChannelSet();
    return mainIn == mainOut && (mainOut == juce::AudioChannelSet::mono()
                              || mainOut == juce::AudioChannelSet::stereo());
}

void TitanTuneMetalAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ignoreUnused (midi);
    juce::ScopedNoDenormals noDenormals;
    const int channels = juce::jmin (buffer.getNumChannels(), 2);
    const int samples = buffer.getNumSamples();

    const float transpose = *parameters.getRawParameterValue ("transpose");
    const float blend = *parameters.getRawParameterValue ("blend");
    const float focus = *parameters.getRawParameterValue ("focus");
    const float gateDb = *parameters.getRawParameterValue ("gate");
    const bool gateOn = parameters.getRawParameterValue ("gateOn")->load() > 0.5f;
    pitchShifter.setSemitones (transpose);

    // Keep the dry signal in the host buffer; process the wet signal separately.
    pitchShifter.process (buffer, shiftedBuffer, 0, samples);

    const float gateThreshold = juce::Decibels::decibelsToGain (gateDb);
    const float gateAttack = std::exp (-1.0f / (0.004f * (float) getSampleRate()));
    const float gateRelease = std::exp (-1.0f / (0.085f * (float) getSampleRate()));

    for (int i = 0; i < samples; ++i)
    {
        for (int ch = 0; ch < channels; ++ch)
        {
            const float x = buffer.getSample (ch, i);
            const float shifted = shiftedBuffer.getSample (ch, i);
            float y = x * (1.0f - blend) + shifted * blend;

            float drive = 1.0f + focus * 2.2f;
            float shaped = std::tanh (y * drive) / std::max (1.0f, std::tanh (drive));
            y = y * (1.0f - focus * 0.28f) + shaped * (focus * 0.28f);

            float& env = (ch == 0 ? gateEnvelopeL : gateEnvelopeR);
            const float absIn = std::abs (y);
            env = absIn > env ? gateAttack * env + (1.0f - gateAttack) * absIn
                              : gateRelease * env + (1.0f - gateRelease) * absIn;
            float gateGain = 1.0f;
            if (gateOn)
            {
                if (env < gateThreshold) gateGain = 0.0f;
                else if (env < gateThreshold * 2.5f)
                    gateGain = (env - gateThreshold) / (gateThreshold * 1.5f);
            }

            if (ch == 0)
            {
                y = highPassL.processSample (0, y);
                y = lowPassL.processSample (0, y);
            }
            else
            {
                y = highPassR.processSample (0, y);
                y = lowPassR.processSample (0, y);
            }
            buffer.setSample (ch, i, y * gateGain);
        }
    }
    for (int ch = channels; ch < buffer.getNumChannels(); ++ch)
        buffer.clear (ch, 0, samples);
}

juce::AudioProcessorEditor* TitanTuneMetalAudioProcessor::createEditor()
{
    return new TitanTuneMetalAudioProcessorEditor (*this);
}

void TitanTuneMetalAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void TitanTuneMetalAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml (getXmlFromBinary (data, sizeInBytes));
    if (xml != nullptr && xml->hasTagName (parameters.state.getType()))
        parameters.replaceState (juce::ValueTree::fromXml (*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new TitanTuneMetalAudioProcessor();
}
