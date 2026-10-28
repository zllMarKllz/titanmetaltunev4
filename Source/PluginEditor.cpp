#include "PluginEditor.h"

TitanTuneMetalAudioProcessorEditor::TitanTuneMetalAudioProcessorEditor (TitanTuneMetalAudioProcessor& p)
    : AudioProcessorEditor (&p), processor (p)
{
    setSize (680, 430);
    titleLabel.setText ("TITANTUNE", juce::dontSendNotification);
    titleLabel.setFont (juce::Font (juce::FontOptions (28.0f, juce::Font::bold)));
    titleLabel.setColour (juce::Label::textColourId, juce::Colour (0xfff2f6f8));
    addAndMakeVisible (titleLabel);

    subtitleLabel.setText ("METAL  /  PITCH ENGINE LAB", juce::dontSendNotification);
    subtitleLabel.setFont (juce::Font (juce::FontOptions (11.0f, juce::Font::bold)));
    subtitleLabel.setColour (juce::Label::textColourId, juce::Colour (0xff55e6ad));
    addAndMakeVisible (subtitleLabel);

    styleSlider (transposeSlider, " st");
    transposeSlider.setRange (-36.0, 12.0, 1.0);
    transposeSlider.setTextValueSuffix (" st");
    transposeSlider.setNumDecimalPlacesToDisplay (0);
    transposeAttachment = std::make_unique<SliderAttachment> (processor.parameters, "transpose", transposeSlider);
    addAndMakeVisible (transposeSlider);
    styleLabel (transposeLabel, "TRANSPOSE", 11.0f);
    addAndMakeVisible (transposeLabel);

    styleSlider (blendSlider, "%");
    blendSlider.setRange (0.0, 1.0, 0.01);
    blendSlider.setTextValueSuffix ("");
    blendAttachment = std::make_unique<SliderAttachment> (processor.parameters, "blend", blendSlider);
    addAndMakeVisible (blendSlider);
    styleLabel (blendLabel, "BLEND", 11.0f);
    addAndMakeVisible (blendLabel);

    styleSlider (focusSlider);
    focusSlider.setRange (0.0, 1.0, 0.01);
    focusAttachment = std::make_unique<SliderAttachment> (processor.parameters, "focus", focusSlider);
    addAndMakeVisible (focusSlider);
    styleLabel (focusLabel, "LOW-END FOCUS", 11.0f);
    addAndMakeVisible (focusLabel);

    styleSlider (gateSlider, " dB");
    gateSlider.setRange (-90.0, -18.0, 1.0);
    gateSlider.setTextValueSuffix (" dB");
    gateAttachment = std::make_unique<SliderAttachment> (processor.parameters, "gate", gateSlider);
    addAndMakeVisible (gateSlider);
    styleLabel (gateLabel, "GATE THRESHOLD", 11.0f);
    addAndMakeVisible (gateLabel);

    gateButton.setColour (juce::ToggleButton::textColourId, juce::Colour (0xffeaf2f2));
    gateButton.setColour (juce::ToggleButton::tickColourId, juce::Colour (0xff55e6ad));
    gateButton.setColour (juce::ToggleButton::tickDisabledColourId, juce::Colour (0xff607078));
    gateAttachmentButton = std::make_unique<ButtonAttachment> (processor.parameters, "gateOn", gateButton);
    addAndMakeVisible (gateButton);
}

void TitanTuneMetalAudioProcessorEditor::styleSlider (juce::Slider& s, const juce::String&)
{
    s.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    s.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 100, 24);
    s.setColour (juce::Slider::rotarySliderFillColourId, juce::Colour (0xff55e6ad));
    s.setColour (juce::Slider::rotarySliderOutlineColourId, juce::Colour (0xff2c4249));
    s.setColour (juce::Slider::thumbColourId, juce::Colour (0xfff2f6f8));
    s.setColour (juce::Slider::textBoxTextColourId, juce::Colour (0xfff2f6f8));
    s.setColour (juce::Slider::textBoxBackgroundColourId, juce::Colour (0xff17262c));
    s.setColour (juce::Slider::textBoxOutlineColourId, juce::Colour (0xff2c4249));
}

void TitanTuneMetalAudioProcessorEditor::styleLabel (juce::Label& l, const juce::String& text, float fontSize)
{
    l.setText (text, juce::dontSendNotification);
    l.setJustificationType (juce::Justification::centred);
    l.setFont (juce::Font (juce::FontOptions (fontSize, juce::Font::bold)));
    l.setColour (juce::Label::textColourId, juce::Colour (0xffa9bcc2));
}

void TitanTuneMetalAudioProcessorEditor::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    g.fillAll (juce::Colour (0xff0b1115));

    juce::ColourGradient bg (juce::Colour (0xff14272d), 0.0f, 0.0f,
                             juce::Colour (0xff0b1115), (float) getWidth(), (float) getHeight(), false);
    g.setGradientFill (bg);
    g.fillRoundedRectangle (bounds.reduced (8.0f), 18.0f);

    g.setColour (juce::Colour (0xff55e6ad).withAlpha (0.8f));
    g.fillRoundedRectangle (18.0f, 18.0f, 4.0f, 58.0f, 2.0f);

    g.setColour (juce::Colour (0xff22363d));
    g.drawRoundedRectangle (bounds.reduced (8.0f), 18.0f, 1.0f);

    g.setColour (juce::Colour (0xff55e6ad).withAlpha (0.08f));
    g.fillEllipse ((float) getWidth() - 170.0f, -90.0f, 260.0f, 260.0f);

    g.setColour (juce::Colour (0xff9db2b9));
    g.setFont (juce::Font (juce::FontOptions (10.0f)));
    g.drawText ("REAL-TIME DSP PROTOTYPE  •  BUILD 0.1", 24, getHeight() - 28, getWidth() - 48, 14,
                juce::Justification::centredLeft);
}

void TitanTuneMetalAudioProcessorEditor::resized()
{
    titleLabel.setBounds (28, 22, 300, 38);
    subtitleLabel.setBounds (30, 57, 300, 20);

    transposeLabel.setBounds (30, 112, 150, 20);
    transposeSlider.setBounds (25, 132, 160, 175);

    blendLabel.setBounds (190, 112, 140, 20);
    blendSlider.setBounds (180, 132, 160, 175);

    focusLabel.setBounds (355, 112, 140, 20);
    focusSlider.setBounds (345, 132, 160, 175);

    gateLabel.setBounds (510, 112, 145, 20);
    gateSlider.setBounds (500, 132, 160, 175);

    gateButton.setBounds (30, 330, 120, 30);
}
