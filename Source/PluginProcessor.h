/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

enum Slope
{
    Slope_12,
    Slope_24,
    Slope_36,
    Slope_48,
    
};

struct ChainSettings
{
    float peakFreq { 0 }, peakGainDecibels{ 0 }, peakQuality { 1.f };
    float highPassFreq { 0 }, lowPassFreq { 0 };
    
    Slope highPassSlope { Slope::Slope_12 }, lowPassSlope { Slope::Slope_12 };
};

ChainSettings getChainSettings(juce::AudioProcessorValueTreeState& apvts);


//==============================================================================
/**
*/
class JhanEQAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    JhanEQAudioProcessor();
    ~JhanEQAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

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
    
    static juce::AudioProcessorValueTreeState::ParameterLayout createrParemterLayout();
    juce::AudioProcessorValueTreeState apvts{*this, nullptr, "Parameters", createrParemterLayout()};

private:
    
    using Filter = juce::dsp::IIR::Filter<float>;
    
    using CutFilter = juce::dsp::ProcessorChain<Filter, Filter, Filter, Filter>;
    
    using MonoChain = juce::dsp::ProcessorChain<CutFilter, Filter, CutFilter>;
    
    MonoChain leftChain, rightChain;
    
    enum ChainPositions
    {
        HighPass,
        Peak,
        LowPass
    };
    
    void updatePeakFilter(const ChainSettings& chainSettings);
    using Coefficients = Filter::CoefficientsPtr;
    static void updateCoefficients(Coefficients& old, const Coefficients& replacements);
    
    template<typename ChainType, typename CoefficientType>
    void updatePassFilter(ChainType& HighPass,
                         const CoefficientType& cutCoefficients,
//                         const ChainSettings& chainSettings)
                          const Slope& highPassSlope)
    
                            
    {
        HighPass.template setBypassed<0>(true);
        HighPass.template setBypassed<1>(true);
        HighPass.template setBypassed<2>(true);
        HighPass.template setBypassed<3>(true);
        
        switch(highPassSlope)
        {
            case Slope_12:
            {
                HighPass.template get<0>().coefficients = cutCoefficients[0];
                HighPass.template setBypassed<0>(false);
                break;
            }
                
            case Slope_24:
            {
                HighPass.template get<0>().coefficients = cutCoefficients[0];
                HighPass.template setBypassed<0>(false);
                HighPass.template get<1>().coefficients = cutCoefficients[1];
                HighPass.template setBypassed<1>(false);
                break;
            }
                
            case Slope_36:
            {
                HighPass.template get<0>().coefficients = cutCoefficients[0];
                HighPass.template setBypassed<0>(false);
                HighPass.template get<1>().coefficients = cutCoefficients[1];
                HighPass.template setBypassed<1>(false);
                HighPass.template get<2>().coefficients = cutCoefficients[2];
                HighPass.template setBypassed<2>(false);
                break;
            }
                
            case Slope_48:
            {
                HighPass.template get<0>().coefficients = cutCoefficients[0];
                HighPass.template setBypassed<0>(false);
                HighPass.template get<1>().coefficients = cutCoefficients[1];
                HighPass.template setBypassed<1>(false);
                HighPass.template get<2>().coefficients = cutCoefficients[2];
                HighPass.template setBypassed<2>(false);
                HighPass.template get<3>().coefficients = cutCoefficients[3];
                HighPass.template setBypassed<3>(false);
                break;
            }
        }
    }
    
    
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (JhanEQAudioProcessor)
};
