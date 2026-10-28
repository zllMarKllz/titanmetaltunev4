#pragma once
#include <JuceHeader.h>
#include "signalsmith-stretch.h"
#include <array>
#include <vector>

class TitanTuneMetalAudioProcessor final : public juce::AudioProcessor
{
public:
    TitanTuneMetalAudioProcessor();
    ~TitanTuneMetalAudioProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.25; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock&) override;
    void setStateInformation (const void*, int) override;

    juce::AudioProcessorValueTreeState parameters;
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

private:
    class PhaseVocoderPitchShifter
    {
    public:
        void prepare (double sr, int maxBlockSize);
        void reset();
        void setSemitones (float st);
        void process (const juce::AudioBuffer<float>& input, juce::AudioBuffer<float>& output,
                      int startSample, int numSamples);
        int getLatencySamples() const noexcept { return latencySamples; }
    private:
        static constexpr int channels = 2;
        signalsmith::stretch::SignalsmithStretch<float> stretcher;
        juce::AudioBuffer<float> inputScratch, outputScratch;
        int scratchSize = 8192;
        int latencySamples = 0;
        float currentSemitones = 0.0f;
        double sampleRate = 44100.0;
    };

    PhaseVocoderPitchShifter pitchShifter;
    juce::AudioBuffer<float> shiftedBuffer;
    juce::dsp::StateVariableTPTFilter<float> highPassL, highPassR;
    juce::dsp::StateVariableTPTFilter<float> lowPassL, lowPassR;
    float gateEnvelopeL = 0.0f, gateEnvelopeR = 0.0f;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TitanTuneMetalAudioProcessor)
};
