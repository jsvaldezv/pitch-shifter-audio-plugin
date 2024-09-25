#pragma once
#include "Vocoder/PitchShifter.h"
#include <JuceHeader.h>

class WangVocoderPitchShifter
{
public:

    WangVocoderPitchShifter() = default;
    ~WangVocoderPitchShifter() = default;

    void prepare (juce::dsp::ProcessSpec&);

    void process (juce::AudioBuffer<float>&);

    void setSemitones (int semitones);

private:

    PitchShifterWangVocoder pitchShifter[2];

    juce::dsp::ProcessSpec spec;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WangVocoderPitchShifter)
};
