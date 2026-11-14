#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
NeonDustLassoAudioProcessorEditor::NeonDustLassoAudioProcessorEditor (NeonDustLassoAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Dark cinematic cyber-western palette: deep black, neon amber, dusty copper
    setSize (720, 420);

    auto setupSlider = [this] (juce::Slider& slider, juce::Label& label, const juce::String& text)
    {
        addAndMakeVisible (slider);
        slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 70, 18);
        slider.setColour (juce::Slider::rotarySliderFillColourId, juce::Colour (0xffc45c26)); // copper
        slider.setColour (juce::Slider::thumbColourId, juce::Colour (0xffffcc66)); // neon amber
        slider.setColour (juce::Slider::textBoxTextColourId, juce::Colours::white);
        slider.setColour (juce::Slider::textBoxBackgroundColourId, juce::Colour (0xff1a1a1a));

        addAndMakeVisible (label);
        label.setText (text, juce::dontSendNotification);
        label.setJustificationType (juce::Justification::centred);
        label.setColour (juce::Label::textColourId, juce::Colour (0xffe0c097));
        label.attachToComponent (&slider, false);
    };

    setupSlider (ratioSlider, ratioLabel, "Ratio");
    setupSlider (indexSlider, indexLabel, "Index");
    setupSlider (cutoffSlider, cutoffLabel, "Cutoff");
    setupSlider (resonanceSlider, resonanceLabel, "Res");
    setupSlider (attackSlider, attackLabel, "Attack");
    setupSlider (decaySlider, decayLabel, "Decay");
    setupSlider (sustainSlider, sustainLabel, "Sustain");
    setupSlider (releaseSlider, releaseLabel, "Release");
    setupSlider (gainSlider, gainLabel, "Gain");

    ratioAttachment     = std::make_unique<Attachment> (audioProcessor.apvts, "ratio", ratioSlider);
    indexAttachment     = std::make_unique<Attachment> (audioProcessor.apvts, "index", indexSlider);
    cutoffAttachment    = std::make_unique<Attachment> (audioProcessor.apvts, "cutoff", cutoffSlider);
    resonanceAttachment = std::make_unique<Attachment> (audioProcessor.apvts, "resonance", resonanceSlider);
    attackAttachment    = std::make_unique<Attachment> (audioProcessor.apvts, "attack", attackSlider);
    decayAttachment     = std::make_unique<Attachment> (audioProcessor.apvts, "decay", decaySlider);
    sustainAttachment   = std::make_unique<Attachment> (audioProcessor.apvts, "sustain", sustainSlider);
    releaseAttachment   = std::make_unique<Attachment> (audioProcessor.apvts, "release", releaseSlider);
    gainAttachment      = std::make_unique<Attachment> (audioProcessor.apvts, "gain", gainSlider);
}

NeonDustLassoAudioProcessorEditor::~NeonDustLassoAudioProcessorEditor() = default;

//==============================================================================
void NeonDustLassoAudioProcessorEditor::paint (juce::Graphics& g)
{
    // Dark cinematic background with subtle gradient and neon accent
    g.fillAll (juce::Colour (0xff0d0d0d));
    juce::ColourGradient gradient (juce::Colour (0xff1a1208), 0, 0,
                                   juce::Colour (0xff0a0a12), getWidth(), getHeight(), false);
    g.setGradientFill (gradient);
    g.fillAll();

    g.setColour (juce::Colour (0xffffaa44));
    g.setFont (juce::FontOptions (28.0f, juce::Font::bold));
    g.drawText ("NEON DUST LASSO", 20, 15, getWidth() - 40, 40, juce::Justification::centredLeft);

    g.setFont (juce::FontOptions (13.0f));
    g.setColour (juce::Colour (0xffb0a080));
    g.drawText ("Experimental Cyber-Western FM  |  MPJRecords  |  2026-11-14", 20, 50, getWidth() - 40, 20, juce::Justification::centredLeft);
}

void NeonDustLassoAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced (20).removeFromBottom (getHeight() - 90);
    auto topRow = area.removeFromTop (180);
    auto bottomRow = area;

    auto place = [] (juce::Slider& s, juce::Rectangle<int> bounds)
    {
        s.setBounds (bounds.reduced (8));
    };

    place (ratioSlider, topRow.removeFromLeft (140));
    place (indexSlider, topRow.removeFromLeft (140));
    place (cutoffSlider, topRow.removeFromLeft (140));
    place (resonanceSlider, topRow.removeFromLeft (140));

    place (attackSlider, bottomRow.removeFromLeft (110));
    place (decaySlider, bottomRow.removeFromLeft (110));
    place (sustainSlider, bottomRow.removeFromLeft (110));
    place (releaseSlider, bottomRow.removeFromLeft (110));
    place (gainSlider, bottomRow.removeFromLeft (110));
}
