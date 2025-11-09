/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.
//PluginEditor.h
  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//knob png
class CustomKnobLookAndFeel : public juce::LookAndFeel_V4
{
public:
    CustomKnobLookAndFeel()
    {
      
        Knobby = juce::ImageCache::getFromMemory(BinaryData::Knobby_png, BinaryData::Knobby_pngSize);

            
    }
    

    void drawRotarySlider(juce::Graphics& g,
                          int x, int y, int width, int height,
                          float sliderPosProportional,
                          float rotaryStartAngle,
                          float rotaryEndAngle,
                          juce::Slider& slider) override
    {
        if (!Knobby.isValid())
            return;
        
        const float fortyFiveDegrees = juce::MathConstants<float>::pi / 4.0f;
        rotaryStartAngle += juce::MathConstants<float>::halfPi + fortyFiveDegrees;
        rotaryEndAngle   += juce::MathConstants<float>::halfPi + fortyFiveDegrees;


        const float rotation = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);
        const float centerX = x + width * 0.5f;
        const float centerY = y + height * 0.5f;

        const float scale = 0.5f;
        juce::AffineTransform t = juce::AffineTransform::identity
                   .translated(-Knobby.getWidth() * 0.5f, -Knobby.getHeight() * 0.5f)
                   .scaled(scale)
                   .rotated(rotation, 0, 0)
                   .translated(centerX, centerY);

        g.drawImageTransformed(Knobby, t);
    }
    
  

private:
    juce::Image Knobby;

    

};











//==============================================================================
/**
*/
class NewProjectAudioProcessorEditor  : public juce::AudioProcessorEditor,
public juce::Slider::Listener, private juce::Timer
{
public:
    NewProjectAudioProcessorEditor (NewProjectAudioProcessor&);
    ~NewProjectAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    void sliderValueChanged(juce::Slider* slider) override;
  

private:
    void timerCallback() override;
    
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    NewProjectAudioProcessor& audioProcessor;
    juce::Slider gainSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> gainAttachment;
    juce::Label gainLabel;
    
    juce::Slider overdriveSlider;
    juce::Label tanhLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> overdriveAttachment;
    
    
  
   
    juce::Label srLabel;
    juce::Slider srSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> targetSRAttachment;
    
    
    juce::Slider carrierSlider;
    juce::Label carrierLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> carrierAttachment;
    
    
    juce::Slider ringMixSlider;
    juce::Label ringMixLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> ringModAmountAttachment;
    
    juce::Slider ringWaveSlider;
    juce::Label ringWaveLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> ringWaveAttachment;
    
    
    
    juce::Image background;
    
    juce::Slider reverbSlider;
    juce::Label reverbLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> reverbAttachment;
    
    
    CustomKnobLookAndFeel customKnobLook;
    
  
    
    
    

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NewProjectAudioProcessorEditor)
};
