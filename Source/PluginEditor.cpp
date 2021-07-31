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
    
    const auto& params = audioProcessor.getParameters();
    for( auto param : params)
    {
        param->addListener(this);
    }
    
    startTimerHz(60);
    
    setSize (600, 400);
}

JhanEQAudioProcessorEditor::~JhanEQAudioProcessorEditor()
{
    const auto& params = audioProcessor.getParameters();
    for( auto param : params)
    {
        param->removeListener(this);
    }
}

//==============================================================================
void JhanEQAudioProcessorEditor::paint (juce::Graphics& g)
{
    
    using namespace juce;
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (Colours::black);
    
    auto bounds = getLocalBounds();
    auto responseArea = bounds.removeFromTop(bounds.getHeight() * 0.33);
    
    auto w = responseArea.getWidth();
    
    auto& highPass = monoChain.get<ChainPositions::HighPass>();
    auto& peak = monoChain.get<ChainPositions::Peak>();
    auto& lowPass = monoChain.get<ChainPositions::LowPass>();
    
    auto sampleRate = audioProcessor.getSampleRate();
    
    std::vector<double> mags;
    
    mags.resize(w);
    
    for ( int i = 0; i < w; ++i) {
        
        double mag = 1.f;
        auto freq = mapToLog10(double(i) / double(w), 20.0, 20000.);
        
        if(! monoChain.isBypassed<ChainPositions::Peak>() )
            mag *= peak.coefficients->getMagnitudeForFrequency(freq, sampleRate);
        
        if( !highPass.isBypassed<0>() )
            mag *= highPass.get<0>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        if( !highPass.isBypassed<1>() )
                   mag *= highPass.get<1>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        if( !highPass.isBypassed<2>() )
                   mag *= highPass.get<2>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        if( !highPass.isBypassed<3>() )
                   mag *= highPass.get<3>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        
        if( !lowPass.isBypassed<0>() )
            mag *= lowPass.get<0>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        if( !lowPass.isBypassed<1>() )
                   mag *= lowPass.get<1>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        if( !lowPass.isBypassed<2>() )
                   mag *= lowPass.get<2>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        if( !lowPass.isBypassed<3>() )
                   mag *= lowPass.get<3>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
            
        mags[i] = Decibels::gainToDecibels(mag);
        
    }
    
    // responseCurve
    
    Path responseCurve;
    
    const double outputMin = responseArea.getBottom();
    const double outputMax = responseArea.getY();
    auto map = [outputMin, outputMax](double input)
    {
        return jmap(input, -24.0, 24.0, outputMin, outputMax);
    };
    
    responseCurve.startNewSubPath(responseArea.getX(), map(mags.front()));
    
    for( size_t i = 1; i < mags.size(); ++i)
    {
        responseCurve.lineTo(responseArea.getX() + i, map(mags[i]));
    }
    
    //set to image
    
    g.setColour(Colours::orange);
    g.drawRoundedRectangle(responseArea.toFloat(), 4.f, 1.f);
    
    g.setColour(Colours::white);
    g.strokePath(responseCurve, PathStrokeType(2.f));
    

    
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

void JhanEQAudioProcessorEditor::parameterValueChanged(int parameterIndex, float newValue)
{
    parametersChanged.set(true);
}

void JhanEQAudioProcessorEditor::timerCallback()
{
    if( parametersChanged.compareAndSetBool(false, true) )
    {
        DBG("params changed: ");
        //update the monochain
        auto chainSettings = getChainSettings(audioProcessor.apvts);
        auto peakCoefficients = makePeakFilter(chainSettings, audioProcessor.getSampleRate());
        updateCoefficients(monoChain.get<ChainPositions::Peak>().coefficients, peakCoefficients);
        
        //signal a repaint
        repaint();
        
        
    }
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
