#pragma once
#include <JuceHeader.h>

class TownleyPitchShifter
{
public:
    
    TownleyPitchShifter() = default;
    ~TownleyPitchShifter() = default;
    
    void fillDelayBuffer (int channel, const int bufferLength, const int delayBufferLength, const float* bufferData, const float* delayBufferData);
        
    void getFromDelayBuffer (juce::AudioBuffer<float>& buffer, int channel, const int bufferLength, const int delayBufferLength, const float* bufferData, const float* delayBufferData);
    
    void prepare (juce::dsp::ProcessSpec& sp);
    
    void process (juce::AudioBuffer<float>&);
    
    void setSemitones (int semitones);
    
private:
    
    juce::AudioBuffer<float> mDelayBuffer; //m denotes member variable
    
    int mWritePosition { 0 };
    
    float currentDelayMS;
    float previousDelayMS;
    
    juce::dsp::ProcessSpec spec;
};
