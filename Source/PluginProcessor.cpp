/*
  ==============================================================================

 //PluginProcessor.c

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"


//==============================================================================
NewProjectAudioProcessor::NewProjectAudioProcessor()
    : AudioProcessor (BusesProperties()
        #if !JucePlugin_IsMidiEffect
         #if !JucePlugin_IsSynth
          .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
         #endif
          .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
        #endif
      )
    , apvts (*this, nullptr, "Parameters", createParameters())
{
    // Initialize DSP variables
    rawVolume = 0.5;
    overdrive = 1.0f;
    counter = 0;
    countTo = 44100;
    targetSR = 44100.f;
    carrier = 440.f;
    carrierPhaseSine = 0.0f;
    carrierPhaseSawtooth = 0.0f;
    ringModAmount = 0.5f;
    ringWave = 0.5f;
    reverbwet = 0.3f;

    // Initialize bitcrusher vectors for all input channels
    bitcrushCounters.resize(getTotalNumInputChannels(), 0);
    storedBitCrushVals.resize(getTotalNumInputChannels(), 0.0f);
}


NewProjectAudioProcessor::~NewProjectAudioProcessor()
{
}

//==============================================================================
const juce::String NewProjectAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool NewProjectAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool NewProjectAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool NewProjectAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double NewProjectAudioProcessor::getTailLengthSeconds() const
{
    return 1.0;
}

int NewProjectAudioProcessor::getNumPrograms()
{
    return 1;
}

int NewProjectAudioProcessor::getCurrentProgram()
{
    return 0;
}


void NewProjectAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String NewProjectAudioProcessor::getProgramName (int index)
{
    return {};
}

void NewProjectAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void NewProjectAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::ignoreUnused(sampleRate, samplesPerBlock);

 
    const int numChannels = std::max(getTotalNumInputChannels(), getTotalNumOutputChannels());
    bitcrushCounters.assign(numChannels, 0);
    storedBitCrushVals.assign(numChannels, 0.0f);

 
    carrierPhaseSine = 0.0f;
    carrierPhaseSawtooth = 0.0f;

    // Initialise classic juce::Reverb params
    juce::Reverb::Parameters rvp;
    rvp.roomSize   = 0.6f;
    rvp.damping    = 0.5f;
    rvp.wetLevel   = 0.3f;
    rvp.dryLevel   = 0.7f;
    rvp.width      = 1.0f;
    rvp.freezeMode = 0.0f;
    reverb.setParameters(rvp);
}

void NewProjectAudioProcessor::releaseResources()
{
}




#ifndef JucePlugin_PreferredChannelConfigurations
bool NewProjectAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else

    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

float NewProjectAudioProcessor::getCurrentSampleRate()
{
    return getSampleRate();
}


juce::AudioProcessorValueTreeState::ParameterLayout NewProjectAudioProcessor::createParameters()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back(std::make_unique<juce::AudioParameterFloat>("gain",
                                                                 "Gain",
                                                                 0.0f,
                                                                 0.7f,
                                                                 0.5f));


    params.push_back(std::make_unique<juce::AudioParameterFloat>("overdrive",
                                                                 "Overdrive",
                                                                 0.1f,
                                                                 10.0f,
                                                                 0.1f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>("targetSR",
                                                                 "TargetSR",
                                                                 0.0f,
                                                                 0.6f,
                                                                 0.6f));


    params.push_back(std::make_unique<juce::AudioParameterFloat>("carrier",
                                                                 "Carrier",
                                                                 20.0f,
                                                                 2000.0f,
                                                                 1000.0f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>("ringWave",
                                                                 "RingWave",
                                                                 0.0,
                                                                 1.0f,
                                                                 0.5f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>("ringModAmount",
                                                                 "ringModAmount",
                                                                 0.0,
                                                                 1.0f,
                                                                 0.0f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>("reverb",
                                                                 "Reverb",
                                                                 0.0,
                                                                 1.0f,
                                                                 0.0f));



    return { params.begin(), params.end() };
}

void NewProjectAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused(midiMessages);
    juce::ScopedNoDenormals noDenormals;

    const int totalNumInputChannels  = getTotalNumInputChannels();
    const int totalNumOutputChannels = getTotalNumOutputChannels();
    const int numChannels = buffer.getNumChannels();
    const int numSamples = buffer.getNumSamples();


    for (int i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, numSamples);


    if ((int)bitcrushCounters.size() != numChannels)
        bitcrushCounters.assign(numChannels, 0);
    if ((int)storedBitCrushVals.size() != numChannels)
        storedBitCrushVals.assign(numChannels, 0.0f);


    const float gainParam           = apvts.getRawParameterValue("gain")->load();
    const float overdriveParam      = apvts.getRawParameterValue("overdrive")->load();
    const float targetSRParam       = apvts.getRawParameterValue("targetSR")->load();
    const float carrierParam        = apvts.getRawParameterValue("carrier")->load();
    const float ringWaveParam       = apvts.getRawParameterValue("ringWave")->load();
    const float ringModAmountParam  = apvts.getRawParameterValue("ringModAmount")->load();
    const float reverbParam         = apvts.getRawParameterValue("reverb")->load();


    {
        juce::Reverb::Parameters rvp;
        rvp.roomSize   = 0.6f;
        rvp.damping    = 0.5f;
        rvp.wetLevel   = reverbParam;
        rvp.dryLevel   = 1.0f - reverbParam;
        rvp.width      = 1.0f;
        rvp.freezeMode = 0.0f;
        reverb.setParameters(rvp);
    }

    const float sampleRateF = static_cast<float>(getSampleRate());
    const float carrierInc = (carrierParam * 2.0f * juce::MathConstants<float>::pi) / sampleRateF;


    float clampedTargetSR = juce::jlimit(0.01f, 0.99f, targetSRParam);
    float desiredHz = sampleRateF * clampedTargetSR;
    countTo = static_cast<int>(std::max(1.0f, sampleRateF / desiredHz));

  
    for (int channel = 0; channel < numChannels; ++channel)
    {
        float* channelData = buffer.getWritePointer(channel);

        for (int sample = 0; sample < numSamples; ++sample)
        {
            float inputSample = channelData[sample];

          
            float distortedSample = std::tanh(inputSample * overdriveParam);
            distortedSample = distortedSample / (overdriveParam / 2.0f);

       
            bitcrushCounters[channel]++;
            if (bitcrushCounters[channel] >= countTo)
            {
                bitcrushCounters[channel] = 0;
                storedBitCrushVals[channel] = distortedSample;
            }
            float crushedSample = storedBitCrushVals[channel] * 0.7f;

        
            carrierPhaseSine += carrierInc;
            if (carrierPhaseSine >= juce::MathConstants<float>::twoPi)
                carrierPhaseSine -= juce::MathConstants<float>::twoPi;

            float carrierSignalSine = std::sin(carrierPhaseSine);

            carrierPhaseSawtooth += (carrierParam / sampleRateF);
            if (carrierPhaseSawtooth >= 1.0f)
                carrierPhaseSawtooth -= 1.0f;

            float sawtoothWave = 2.0f * carrierPhaseSawtooth - 1.0f;

            float modulatedSineSample     = crushedSample * carrierSignalSine;
            float modulatedSawtoothSample = crushedSample * sawtoothWave;

            float modulatedSample =
                (1.0f - ringWaveParam) * modulatedSineSample
              + ringWaveParam         * modulatedSawtoothSample;

            float outputSample =
                (1.0f - ringModAmountParam) * crushedSample
              + ringModAmountParam     * modulatedSample;

            
            channelData[sample] = outputSample * gainParam;
        }
    }


    if (numChannels == 1)
    {
        reverb.processMono(buffer.getWritePointer(0), numSamples);
    }
    else if (numChannels >= 2)
    {
        reverb.processStereo(buffer.getWritePointer(0),
                             buffer.getWritePointer(1),
                             numSamples);
       
    }
}



//==============================================================================
bool NewProjectAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* NewProjectAudioProcessor::createEditor()
{
    return new NewProjectAudioProcessorEditor (*this);
}

//==============================================================================
void NewProjectAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void NewProjectAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState != nullptr)
        apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

//==============================================================================

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new NewProjectAudioProcessor();
}

