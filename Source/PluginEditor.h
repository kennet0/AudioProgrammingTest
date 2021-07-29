/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

struct CustomRotarySlider : juce::Slider
{
    CustomRotarySlider() : juce::Slider(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag,
                                        juce::Slider::TextEntryBoxPosition::NoTextBox)
    {
        
    }
};

//==============================================================================
/**
*/
class JhanEQAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    JhanEQAudioProcessorEditor (JhanEQAudioProcessor&);
    ~JhanEQAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    JhanEQAudioProcessor& audioProcessor;
    
    CustomRotarySlider peakFreqSlider,
                        peakGainSlider,
                        peakQualitySlider,
                        highPassFreqSlider,
                        lowPassFreqSiler,
                        highPassSlopeSlider,
                        lowPassSlopeSlider;
    
    using APVTS = juce::AudioProcessorValueTreeState;
    using Attachment = APVTS::SliderAttachment;
    
    Attachment peakFreqSliderAttachment,
                peakGainSliderAttachment,
                peakQualitySliderAttachment,
                highPassFreqSliderAttachment,
                lowPassFreqSilerAttachment,
                highPassSlopeSliderAttachment,
                lowPassSlopeSliderAttachment;
            
    
    std::vector<juce::Component*> getComps();
    
    MonoChain monoChain;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (JhanEQAudioProcessorEditor)
};
