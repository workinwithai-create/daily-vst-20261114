#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
class NeonDustLassoAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    NeonDustLassoAudioProcessorEditor (NeonDustLassoAudioProcessor&);
    ~NeonDustLassoAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    NeonDustLassoAudioProcessor& audioProcessor;

    juce::Slider ratioSlider, indexSlider, cutoffSlider, resonanceSlider;
    juce::Slider attackSlider, decaySlider, sustainSlider, releaseSlider, gainSlider;

    juce::Label ratioLabel, indexLabel, cutoffLabel, resonanceLabel;
    juce::Label attackLabel, decayLabel, sustainLabel, releaseLabel, gainLabel;

    using Attachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    std::unique_ptr<Attachment> ratioAttachment, indexAttachment, cutoffAttachment, resonanceAttachment;
    std::unique_ptr<Attachment> attackAttachment, decayAttachment, sustainAttachment, releaseAttachment, gainAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NeonDustLassoAudioProcessorEditor)
};
