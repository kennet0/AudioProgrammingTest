/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
JhanEQAudioProcessorEditor::JhanEQAudioProcessorEditor (JhanEQAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p),
peakFreqSliderAttachment(audioProcessor.apvts, "Peak Freq", peakFreqSlider),
peakGainSliderAttachment(audioProcessor.apvts, "Peak Gain", peakGainSlider),
peakQualitySliderAttachment(audioProcessor.apvts, "Peak Quality", peakQualitySlider),
highPassFreqSliderAttachment(audioProcessor.apvts, "HighPass Freq", highPassFreqSlider),
lowPassFreqSilerAttachment(audioProcessor.apvts, "LowPass Freq", lowPassFreqSiler),
highPassSlopeSliderAttachment(audioProcessor.apvts, "HighPass Slope", highPassSlopeSlider),
lowPassSlopeSliderAttachment(audioProcessor.apvts, "LowPass Slope", lowPassSlopeSlider)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    
    for( auto* comp : getComps() )
    {
        addAndMakeVisible(comp);
    }
    
    setSize (600, 400);
}

JhanEQAudioProcessorEditor::~JhanEQAudioProcessorEditor()
{
}

//==============================================================================
void JhanEQAudioProcessorEditor::paint (juce::Graphics& g)
{
    
    using namespace juce;
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (Colours::black);
    
    //1:42:16
    


    
}

void JhanEQAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    auto bounds = getLocalBounds();
    auto responseArea = bounds.removeFromTop(bounds.getHeight() * 0.33);
    
    auto highPassArea = bounds.removeFromLeft(bounds.getWidth() * 0.33);
    auto lowPassArea = bounds.removeFromRight(bounds.getWidth() * 0.5);
    
    highPassFreqSlider.setBounds(highPassArea.removeFromTop(highPassArea.getHeight() * 0.5));
    highPassSlopeSlider.setBounds(highPassArea);
    
    lowPassFreqSiler.setBounds(lowPassArea.removeFromTop(lowPassArea.getHeight() * 0.5));
    lowPassSlopeSlider.setBounds(lowPassArea);
                               
    peakFreqSlider.setBounds(bounds.removeFromTop(bounds.getHeight() * 0.33));
    peakGainSlider.setBounds(bounds.removeFromTop(bounds.getHeight() * 0.5));
    peakQualitySlider.setBounds(bounds);
     
}

std::vector<juce::Component*> JhanEQAudioProcessorEditor::getComps()
{
    return
    {
        &peakFreqSlider,
        &peakGainSlider,
        &peakQualitySlider,
        &highPassFreqSlider,
        &lowPassFreqSiler,
        &highPassSlopeSlider,
        &lowPassSlopeSlider
        
    };
}
