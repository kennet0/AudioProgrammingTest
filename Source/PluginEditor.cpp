/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

void LookAndFeel::drawRotarySlider(juce::Graphics &g,
                                   int x,
                                   int y,
                                   int width,
                                   int height,
                                   float sliderPosProportional,
                                   float rotaryStartAngle,
                                   float rotaryEndAngle,
                                   juce::Slider &slider)
{
    using namespace juce;
    
    auto bounds = Rectangle<float>(x, y, width, height);
    
    g.setColour(Colour(137u, 203u, 178u));
    g.fillEllipse(bounds);
    
    g.setColour(Colour(233u, 97u, 100u));
    g.drawEllipse(bounds, 1.f);
    
    if( auto* rswl = dynamic_cast<RotarySliderWithLabels* >(&slider))
    {
        auto center = bounds.getCentre();
        Path p;
        
        Rectangle<float> r;
        r.setLeft(center.getX() - 2);
        r.setRight(center.getX() + 2);
        r.setTop(bounds.getY());
        r.setBottom(center.getY() - rswl->getTextHeight() * 1.5 );
        
        
        p.addRoundedRectangle(r, 2.f);
        
        jassert(rotaryStartAngle < rotaryEndAngle);
        
        auto sliderAngRad = jmap(sliderPosProportional, 0.f, 1.f, rotaryStartAngle, rotaryEndAngle);
        
        p.applyTransform(AffineTransform().rotated(sliderAngRad, center.getX(), center.getY()));
        
        g.setColour(Colour(0u, 97u, 100u));
        g.fillPath(p);
        
        g.setFont(rswl->getTextHeight());
        auto text = rswl->getDisplayString();
        auto strWidth = g.getCurrentFont().getStringWidth(text);
        
        r.setSize(strWidth + 4, rswl->getTextHeight() + 2);
        r.setCentre(bounds.getCentre());
        
        g.setColour(Colours::black);
        g.fillRect(r);
        
        g.setColour(Colours::white);
        g.drawFittedText(text, r.toNearestInt(), juce::Justification::centred, 1);
        
        
    }
    
}

juce::String RotarySliderWithLabels::getDisplayString() const
{
    if( auto* choiceParam = dynamic_cast<juce::AudioParameterChoice*>(param) )
        return choiceParam->getCurrentChoiceName();
    
    juce::String str;
    bool addK = false;
    
    if( auto* floatParam = dynamic_cast<juce::AudioParameterFloat*>(param) )
    {
        float val = getValue();
        
        if( val > 999.f)
        {
            val /= 1000.f;
            addK = true;
        }
        
        str = juce::String(val, (addK ? 2 : 0));
    }
    else
    {
        jassertfalse;
    }
    
    if( suffix.isNotEmpty() )
    {
        str << " " ;
        if( addK )
            str << "k";
        
        str << suffix;
    }
    
    return str;
   
        
}
//==========================================================================================
void RotarySliderWithLabels::paint(juce::Graphics &g)
{
    using namespace juce;
    
    auto startAng = degreesToRadians(180.f + 45.f);
    auto endAng = degreesToRadians(180.f - 45.f) + MathConstants<float>::twoPi;
    
    auto range = getRange();
    
    auto sliderBounds = getSliderBounds();
    
    //Square boundind boxes
//    g.setColour(Colours::red);
//    g.drawRect(getLocalBounds());
//    g.setColour(Colours::yellow);
//    g.drawRect(sliderBounds);
    
    getLookAndFeel().drawRotarySlider(g,
                                      sliderBounds.getX(),
                                      sliderBounds.getY(),
                                      sliderBounds.getWidth(),
                                      sliderBounds.getHeight(),
                                      jmap(getValue(), range.getStart(), range.getEnd(), 0.0, 1.0),
                                      startAng,
                                      endAng,
                                      *this);
    
    
    auto center = sliderBounds.toFloat().getCentre();
    auto radius = sliderBounds.getWidth() * 0.5f;
    
    g.setColour(Colour(229u, 86u, 153u));
    g.setFont(getTextHeight());
    
    auto numChoices = labels.size();
    for( int i = 0; i < numChoices; ++i)
    {
        auto pos = labels[i].pos;
        jassert(0.f <= pos);
        jassert(pos <= 1.f);
        
        auto ang =jmap(pos, 0.f, 1.f, startAng, endAng);
        
        auto c = center.getPointOnCircumference(radius + getTextHeight() * 0.5 + 1, ang);
        
        Rectangle<float> r;
        auto str = labels[i].label;
        r.setSize(g.getCurrentFont().getStringWidth(str), getTextHeight());
        r.setCentre(c);
        r.setY(r.getY() + getTextHeight());
        
        g.drawFittedText(str, r.toNearestInt(), juce::Justification::centred, 1);
        
    }
    
    
}

juce::Rectangle<int> RotarySliderWithLabels::getSliderBounds() const
{
    auto bounds = getLocalBounds();
    
    auto size = juce::jmin(bounds.getWidth(), bounds.getHeight());
    
    size -= getTextHeight() * 2;
    
    juce::Rectangle<int> r;
    r.setSize(size, size);
    r.setCentre(bounds.getCentreX(), 0);
    r.setY(2);
    
    return r;
    
}

//==========================================================================================
ResponseCurveComponent::ResponseCurveComponent(JhanEQAudioProcessor& p) : audioProcessor(p)
{
    const auto& params = audioProcessor.getParameters();
       for( auto param : params)
       {
           param->addListener(this);
       }
    
    updateChain();
    
    startTimerHz(60);
}

ResponseCurveComponent::~ResponseCurveComponent()
{
    const auto& params = audioProcessor.getParameters();
    for( auto param : params)
    {
        param->removeListener(this);
    }
}

void ResponseCurveComponent::parameterValueChanged(int parameterIndex, float newValue)
{
    parametersChanged.set(true);
}

void ResponseCurveComponent::timerCallback()
{
    if( parametersChanged.compareAndSetBool(false, true) )
    {
        DBG("params changed: ");
        //update the monochain
        updateChain();
        //signal a repaint
        repaint();
    }
}

void ResponseCurveComponent::updateChain()
{
    auto chainSettings = getChainSettings(audioProcessor.apvts);
    
    auto peakCoefficients = makePeakFilter(chainSettings, audioProcessor.getSampleRate());
    updateCoefficients(monoChain.get<ChainPositions::Peak>().coefficients, peakCoefficients);
    
    auto highPassCoefficients = makeHighPassFilter(chainSettings, audioProcessor.getSampleRate());
    auto lowPassCoefficients = makeLowPassFilter(chainSettings, audioProcessor.getSampleRate());
    updatePassFilter(monoChain.get<ChainPositions::HighPass>(), highPassCoefficients, chainSettings.highPassSlope);
    updatePassFilter(monoChain.get<ChainPositions::LowPass>(), lowPassCoefficients, chainSettings.lowPassSlope);
    
}

void ResponseCurveComponent::paint (juce::Graphics& g)
{
    
    using namespace juce;
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (Colours::black);
    
    auto responseArea = getLocalBounds();
    
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

//==============================================================================
JhanEQAudioProcessorEditor::JhanEQAudioProcessorEditor (JhanEQAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p),

peakFreqSlider(*audioProcessor.apvts.getParameter("Peak Freq"), "Hz"),
peakGainSlider(*audioProcessor.apvts.getParameter("Peak Gain"), "dB"),
peakQualitySlider(*audioProcessor.apvts.getParameter("Peak Quality"), ""),
highPassFreqSlider(*audioProcessor.apvts.getParameter("HighPass Freq"), "Hz"),
lowPassFreqSlider(*audioProcessor.apvts.getParameter("LowPass Freq"), "Hz"),
highPassSlopeSlider(*audioProcessor.apvts.getParameter("HighPass Slope"), "dB/Oct"),
lowPassSlopeSlider(*audioProcessor.apvts.getParameter("LowPass Slope"), "dB/Oct"),

responseCurveComponent(audioProcessor),
peakFreqSliderAttachment(audioProcessor.apvts, "Peak Freq", peakFreqSlider),
peakGainSliderAttachment(audioProcessor.apvts, "Peak Gain", peakGainSlider),
peakQualitySliderAttachment(audioProcessor.apvts, "Peak Quality", peakQualitySlider),
highPassFreqSliderAttachment(audioProcessor.apvts, "HighPass Freq", highPassFreqSlider),
lowPassFreqSliderAttachment(audioProcessor.apvts, "LowPass Freq", lowPassFreqSlider),
highPassSlopeSliderAttachment(audioProcessor.apvts, "HighPass Slope", highPassSlopeSlider),
lowPassSlopeSliderAttachment(audioProcessor.apvts, "LowPass Slope", lowPassSlopeSlider)

{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    
    peakFreqSlider.labels.add({0.f, "20Hz"});
    peakFreqSlider.labels.add({1.f, "20kHz"});
    
    peakGainSlider.labels.add({0.f,"-24dB"});
    peakGainSlider.labels.add({1.f,"+24dB"});
    
    peakQualitySlider.labels.add({0.f, "0.1"});
    peakQualitySlider.labels.add({1.f, "10.0"});
    
    highPassFreqSlider.labels.add({0.f, "20Hz"});
    highPassFreqSlider.labels.add({1.f, "20kHz"});
    
    lowPassFreqSlider.labels.add({0.f, "20Hz"});
    lowPassFreqSlider.labels.add({1.f, "20kHz"});
    
    highPassSlopeSlider.labels.add({0.f, "12"});
    highPassSlopeSlider.labels.add({1.f, "24"});
    
    lowPassSlopeSlider.labels.add({0.f, "12"});
    lowPassSlopeSlider.labels.add({1.f, "24"});
    
    for( auto* comp : getComps() )
    {
        addAndMakeVisible(comp);
    }

    setSize (600, 480);
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
    
    
}

void JhanEQAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    auto bounds = getLocalBounds();
    float hRatio = 25.f / 100.f;
    auto responseArea = bounds.removeFromTop(bounds.getHeight() * hRatio);
    
    responseCurveComponent.setBounds(responseArea);
    
    bounds.removeFromTop(5);
    
    auto highPassArea = bounds.removeFromLeft(bounds.getWidth() * 0.33);
    auto lowPassArea = bounds.removeFromRight(bounds.getWidth() * 0.5);
    
    highPassFreqSlider.setBounds(highPassArea.removeFromTop(highPassArea.getHeight() * 0.5));
    highPassSlopeSlider.setBounds(highPassArea);

    lowPassFreqSlider.setBounds(lowPassArea.removeFromTop(lowPassArea.getHeight() * 0.5));
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
        &lowPassFreqSlider,
        &highPassSlopeSlider,
        &lowPassSlopeSlider,
        &responseCurveComponent
        
    };
}
