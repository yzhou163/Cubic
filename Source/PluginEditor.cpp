/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.
 //PluginEditor.c

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
NewProjectAudioProcessorEditor::NewProjectAudioProcessorEditor (NewProjectAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
  
    setSize (720, 370);
    
    
    gainSlider.setSliderStyle(juce::Slider::SliderStyle::LinearHorizontal);
    gainSlider.setRange(0.0, 0.7, 0.01);
   gainSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    gainSlider.setValue(0.5);
   
    gainSlider.setBounds(60, -118, 90, 300);
    gainSlider.setColour(juce::Slider::thumbColourId, juce::Colours::black);
    addAndMakeVisible(gainSlider);
    gainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.apvts, "gain", gainSlider);

    //tanh
    overdriveSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
       overdriveSlider.setRange(0.1, 10.0, 0.01); // Min: 0.1, Max: 10, Step size: 0.01
      overdriveSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
      
    overdriveSlider.setValue(0.1);
    overdriveSlider.setBounds(210, 13, 50, 225);
       addAndMakeVisible(overdriveSlider);  // Add the slider
    overdriveAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.apvts, "overdrive", overdriveSlider);
    overdriveSlider.setLookAndFeel(&customKnobLook);


   
    
    srSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
       srSlider.setRange(0, 0.6, 0.01); // Min: 0.1, Max: 10, Step size: 0.01
       srSlider.setValue(0.6); // Default value for sr threshold
      srSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
      
    srSlider.setBounds(335, 25, 50, 225);
       addAndMakeVisible(srSlider);  // Add the slider
    targetSRAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.apvts, "targetSR", srSlider);
    srSlider.setLookAndFeel(&customKnobLook);
    

//    
    //ring modulator frequency
    carrierSlider.setSliderStyle(juce::Slider::SliderStyle::LinearHorizontal);
    carrierSlider.setRange(20.0f, 2000.0f, 1.0f);
    carrierSlider.setValue(1000.0f); //
    carrierSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    carrierSlider.setColour(juce::Slider::thumbColourId, juce::Colours::black);
   
    carrierSlider.setBounds(615, -118, 80, 300);
    addAndMakeVisible(carrierSlider);
    carrierAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.apvts, "carrier", carrierSlider);

    

    //ring mix //ringmodamount
    
    ringMixSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    ringMixSlider.setRange(0.f, 1.f, 0.1f); // Range between 20 Hz and 2000 Hz
    ringMixSlider.setValue(0.0f);
    ringMixSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
   
    ringMixSlider.setBounds(270, 160, 50, 225);
    addAndMakeVisible(ringMixSlider);
    ringModAmountAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.apvts, "ringModAmount", ringMixSlider);
    ringMixSlider.setLookAndFeel(&customKnobLook);
    
    
    //ring wavemix
    
    ringWaveSlider.setSliderStyle(juce::Slider::SliderStyle::LinearHorizontal);
    ringWaveSlider.setRange(0.f, 1.f, 0.1f); // Range between 20 Hz and 2000 Hz
    ringWaveSlider.setValue(0.5f); // Default value (A4 note, 440 Hz)
   ringWaveSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
  
    ringWaveSlider.setColour(juce::Slider::thumbColourId, juce::Colours::black);
    ringWaveSlider.setBounds(615, 202, 80, 300);
    addAndMakeVisible(ringWaveSlider);
    ringWaveAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.apvts, "ringWave", ringWaveSlider);

 
    //reverb wet
    reverbSlider.setSliderStyle(juce::Slider::SliderStyle::LinearHorizontal);
    reverbSlider.setRange(0.f, 1.f, 0.1f); // Range between 20 Hz and 2000 Hz
    reverbSlider.setValue(0.0f); // Default value (A4 note, 440 Hz)
  reverbSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
   
    reverbSlider.setColour(juce::Slider::thumbColourId, juce::Colours::black);
    reverbSlider.setBounds(60, 202, 90, 300);
    addAndMakeVisible(reverbSlider);

    
    reverbAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.apvts, "reverb", reverbSlider);
    reverbSlider.setInterceptsMouseClicks(true, true);
    

    startTimerHz(30);
}


NewProjectAudioProcessorEditor::~NewProjectAudioProcessorEditor()
{  juce::Desktop::getInstance().getDefaultLookAndFeel().setColour(juce::Slider::thumbColourId, juce::Colours::black);
    overdriveSlider.setLookAndFeel(nullptr);
    srSlider.setLookAndFeel(nullptr);
    ringMixSlider.setLookAndFeel(nullptr);
    stopTimer();
}

//==============================================================================
void NewProjectAudioProcessorEditor::paint (juce::Graphics& g)
{
  
    auto background = juce::ImageCache::getFromMemory(BinaryData::CCCubic_png, BinaryData::CCCubic_pngSize);
       g.drawImageWithin(background, 0, 0, getWidth(), getHeight(), juce::RectanglePlacement::stretchToFit);
}

void NewProjectAudioProcessorEditor::resized()
{

    gainSlider.setBounds(60, -118, 90, 300);
      overdriveSlider.setBounds(210, 13, 50, 225);
      srSlider.setBounds(335, 25, 50, 225);
      carrierSlider.setBounds(615, -118, 80, 300);
      ringMixSlider.setBounds(270, 160, 50, 225);
      ringWaveSlider.setBounds(615, 202, 80, 300);
      reverbSlider.setBounds(60, 202, 90, 300);
    
}

void NewProjectAudioProcessorEditor::sliderValueChanged (juce::Slider* slider)
{
  
}

void NewProjectAudioProcessorEditor::timerCallback()
{
    juce::AudioPlayHead::CurrentPositionInfo pos;
    
    if (auto* playHead = audioProcessor.getPlayHead())
    {
        if (playHead->getCurrentPosition(pos))
        {
            
        }
        
        
        
    }}

