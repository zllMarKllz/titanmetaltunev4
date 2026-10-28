#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class TitanTuneMetalAudioProcessorEditor final : public juce::AudioProcessorEditor
{
public:
    explicit TitanTuneMetalAudioProcessorEditor (TitanTuneMetalAudioProcessor&);
    ~TitanTuneMetalAudioProcessorEditor() override = default;
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    TitanTuneMetalAudioProcessor& processor;
    juce::Slider transposeSlider, blendSlider, focusSlider, gateSlider;
    juce::ToggleButton gateButton { "GATE" };
    juce::Label titleLabel, subtitleLabel, transposeLabel, blendLabel, focusLabel, gateLabel;
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;
    std::unique_ptr<SliderAttachment> transposeAttachment, blendAttachment, focusAttachment, gateAttachment;
    std::unique_ptr<ButtonAttachment> gateAttachmentButton;

    void styleSlider (juce::Slider&, const juce::String& suffix = {});
    void styleLabel (juce::Label&, const juce::String&, float fontSize);
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TitanTuneMetalAudioProcessorEditor)
};
