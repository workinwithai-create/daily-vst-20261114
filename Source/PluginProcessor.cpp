#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
NeonDustLassoAudioProcessor::NeonDustLassoAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
#endif
       apvts (*this, nullptr, "Parameters", createParameterLayout())
{
    ratioSmoothed.reset (sampleRate, 0.05);
    indexSmoothed.reset (sampleRate, 0.05);
    cutoffSmoothed.reset (sampleRate, 0.05);
    resonanceSmoothed.reset (sampleRate, 0.05);
    gainSmoothed.reset (sampleRate, 0.05);
}

NeonDustLassoAudioProcessor::~NeonDustLassoAudioProcessor() = default;

//==============================================================================
const juce::String NeonDustLassoAudioProcessor::getName() const { return JucePlugin_Name; }
bool NeonDustLassoAudioProcessor::acceptsMidi() const { return true; }
bool NeonDustLassoAudioProcessor::producesMidi() const { return false; }
bool NeonDustLassoAudioProcessor::isMidiEffect() const { return false; }
double NeonDustLassoAudioProcessor::getTailLengthSeconds() const { return 0.0; }
int NeonDustLassoAudioProcessor::getNumPrograms() { return 1; }
int NeonDustLassoAudioProcessor::getCurrentProgram() { return 0; }
void NeonDustLassoAudioProcessor::setCurrentProgram (int) {}
const juce::String NeonDustLassoAudioProcessor::getProgramName (int) { return {}; }
void NeonDustLassoAudioProcessor::changeProgramName (int, const juce::String&) {}

//==============================================================================
void NeonDustLassoAudioProcessor::prepareToPlay (double newSampleRate, int samplesPerBlock)
{
    sampleRate = newSampleRate;

    ratioSmoothed.reset (sampleRate, 0.05);
    indexSmoothed.reset (sampleRate, 0.05);
    cutoffSmoothed.reset (sampleRate, 0.05);
    resonanceSmoothed.reset (sampleRate, 0.05);
    gainSmoothed.reset (sampleRate, 0.05);

    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32> (samplesPerBlock);
    spec.numChannels = 2;

    filter.prepare (spec);
    filter.setType (juce::dsp::StateVariableTPTFilterType::lowpass);

    limiter.prepare (spec);
    limiter.setThreshold (-1.0f);
    limiter.setRelease (50.0f);

    voice.adsr.setSampleRate (sampleRate);
    adsrParams.attack  = 0.01f;
    adsrParams.decay   = 0.3f;
    adsrParams.sustain = 0.7f;
    adsrParams.release = 0.4f;
    voice.adsr.setParameters (adsrParams);
}

void NeonDustLassoAudioProcessor::releaseResources() {}

bool NeonDustLassoAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    return true;
}

void NeonDustLassoAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    buffer.clear();

    // Update smoothed targets from APVTS (real-time safe)
    ratioSmoothed.setTargetValue (*apvts.getRawParameterValue ("ratio"));
    indexSmoothed.setTargetValue (*apvts.getRawParameterValue ("index"));
    cutoffSmoothed.setTargetValue (*apvts.getRawParameterValue ("cutoff"));
    resonanceSmoothed.setTargetValue (*apvts.getRawParameterValue ("resonance"));
    gainSmoothed.setTargetValue (*apvts.getRawParameterValue ("gain"));

    // Update ADSR from params
    adsrParams.attack  = *apvts.getRawParameterValue ("attack");
    adsrParams.decay   = *apvts.getRawParameterValue ("decay");
    adsrParams.sustain = *apvts.getRawParameterValue ("sustain");
    adsrParams.release = *apvts.getRawParameterValue ("release");
    voice.adsr.setParameters (adsrParams);

    // Process MIDI
    for (const auto metadata : midiMessages)
    {
        const auto msg = metadata.getMessage();
        if (msg.isNoteOn())
        {
            voice.active = true;
            voice.midiNote = msg.getNoteNumber();
            voice.velocity = msg.getFloatVelocity();
            voice.carrierPhase = 0.0;
            voice.modulatorPhase = 0.0;
            voice.adsr.noteOn();
        }
        else if (msg.isNoteOff() && msg.getNoteNumber() == voice.midiNote)
        {
            voice.adsr.noteOff();
        }
    }

    const int numSamples = buffer.getNumSamples();
    auto* left = buffer.getWritePointer (0);
    auto* right = totalNumOutputChannels > 1 ? buffer.getWritePointer (1) : left;

    for (int i = 0; i < numSamples; ++i)
    {
        float sample = 0.0f;

        if (voice.active)
        {
            const float ratio = ratioSmoothed.getNextValue();
            const float index = indexSmoothed.getNextValue();
            const float cutoff = cutoffSmoothed.getNextValue();
            const float resonance = resonanceSmoothed.getNextValue();

            const double freq = juce::MidiMessage::getMidiNoteInHertz (voice.midiNote);
            const double carrierInc = (2.0 * juce::MathConstants<double>::pi * freq) / sampleRate;
            const double modInc = carrierInc * ratio;

            const double mod = std::sin (voice.modulatorPhase) * index;
            sample = static_cast<float> (std::sin (voice.carrierPhase + mod) * voice.velocity * voice.adsr.getNextSample());

            voice.carrierPhase += carrierInc;
            voice.modulatorPhase += modInc;
            if (voice.carrierPhase > juce::MathConstants<double>::twoPi) voice.carrierPhase -= juce::MathConstants<double>::twoPi;
            if (voice.modulatorPhase > juce::MathConstants<double>::twoPi) voice.modulatorPhase -= juce::MathConstants<double>::twoPi;

            if (! voice.adsr.isActive())
                voice.active = false;

            filter.setCutoffFrequency (cutoff);
            filter.setResonance (resonance);
        }

        // Simple mono filter process
        float filtered = filter.processSample (0, sample);
        filtered *= gainSmoothed.getNextValue();

        left[i] = filtered;
        if (totalNumOutputChannels > 1)
            right[i] = filtered;
    }

    // Safety limiter
    juce::dsp::AudioBlock<float> block (buffer);
    juce::dsp::ProcessContextReplacing<float> context (block);
    limiter.process (context);
}

//==============================================================================
bool NeonDustLassoAudioProcessor::hasEditor() const { return true; }

juce::AudioProcessorEditor* NeonDustLassoAudioProcessor::createEditor()
{
    return new NeonDustLassoAudioProcessorEditor (*this);
}

//==============================================================================
void NeonDustLassoAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void NeonDustLassoAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    if (xmlState != nullptr && xmlState->hasTagName (apvts.state.getType()))
        apvts.replaceState (juce::ValueTree::fromXml (*xmlState));
}

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout NeonDustLassoAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add (std::make_unique<juce::AudioParameterFloat> ("ratio", "Carrier Ratio", juce::NormalisableRange<float> (0.5f, 8.0f, 0.01f), 2.0f));
    layout.add (std::make_unique<juce::AudioParameterFloat> ("index", "Mod Index", juce::NormalisableRange<float> (0.0f, 12.0f, 0.01f), 3.0f));
    layout.add (std::make_unique<juce::AudioParameterFloat> ("cutoff", "Cutoff", juce::NormalisableRange<float> (20.0f, 20000.0f, 1.0f, 0.3f), 5000.0f));
    layout.add (std::make_unique<juce::AudioParameterFloat> ("resonance", "Resonance", juce::NormalisableRange<float> (0.1f, 10.0f, 0.01f), 0.7f));
    layout.add (std::make_unique<juce::AudioParameterFloat> ("attack", "Attack", juce::NormalisableRange<float> (0.001f, 2.0f, 0.001f), 0.02f));
    layout.add (std::make_unique<juce::AudioParameterFloat> ("decay", "Decay", juce::NormalisableRange<float> (0.001f, 2.0f, 0.001f), 0.3f));
    layout.add (std::make_unique<juce::AudioParameterFloat> ("sustain", "Sustain", juce::NormalisableRange<float> (0.0f, 1.0f, 0.01f), 0.7f));
    layout.add (std::make_unique<juce::AudioParameterFloat> ("release", "Release", juce::NormalisableRange<float> (0.001f, 3.0f, 0.001f), 0.5f));
    layout.add (std::make_unique<juce::AudioParameterFloat> ("gain", "Gain", juce::NormalisableRange<float> (0.0f, 1.0f, 0.01f), 0.5f));

    return layout;
}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new NeonDustLassoAudioProcessor();
}
