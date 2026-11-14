#pragma once

#include <JuceHeader.h>

//==============================================================================
class NeonDustLassoAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    NeonDustLassoAudioProcessor();
    ~NeonDustLassoAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    //==============================================================================
    juce::AudioProcessorValueTreeState apvts;

private:
    //==============================================================================
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    struct Voice
    {
        bool active = false;
        int midiNote = -1;
        float velocity = 0.0f;
        double carrierPhase = 0.0;
        double modulatorPhase = 0.0;
        juce::ADSR adsr;
    };

    Voice voice;
    double sampleRate = 44100.0;

    juce::SmoothedValue<float> ratioSmoothed;
    juce::SmoothedValue<float> indexSmoothed;
    juce::SmoothedValue<float> cutoffSmoothed;
    juce::SmoothedValue<float> resonanceSmoothed;
    juce::SmoothedValue<float> gainSmoothed;

    juce::dsp::StateVariableTPTFilter<float> filter;
    juce::dsp::Limiter<float> limiter;

    juce::ADSR::Parameters adsrParams;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NeonDustLassoAudioProcessor)
};
