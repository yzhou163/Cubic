#pragma once

#include <JuceHeader.h>

//==============================================================================
class NewProjectAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    NewProjectAudioProcessor();
    ~NewProjectAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorValueTreeState apvts;
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameters();

    
    double rawVolume = 0.5;
    float overdrive = 1.0f;
    int counter = 0;
    int countTo = 44100;
    float targetSR = 44100.f;
    float carrier = 440.f;
    float carrierPhaseSine = 0.0f;
    float carrierPhaseSawtooth = 0.0f;
    float ringModAmount = 0.5f;
    float ringWave = 0.5f;
    float reverbwet = 0.3f;


    std::vector<int> bitcrushCounters;
    std::vector<float> storedBitCrushVals;
    std::vector<char> bitcrushInitialized;

    std::vector<float> carrierPhaseSinePerChannel;
    std::vector<float> carrierPhaseSawtoothPerChannel;
    juce::Reverb dspReverb; 

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

    float getCurrentSampleRate();

private:
    float gain = 1.0f;

    juce::Reverb reverb;
    juce::Reverb::Parameters reverbParams;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NewProjectAudioProcessor)
};

