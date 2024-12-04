#pragma once
#include "PitchShifter.h"
#include <JuceHeader.h>

class DysomniPitchShifter
{
public:

    DysomniPitchShifter() = default;
    ~DysomniPitchShifter() = default;

    void prepare (juce::dsp::ProcessSpec&);

    void process (juce::AudioBuffer<float>&);

    void setSemitones (int semitones);

private:

    static const int harmonies { 5 };

    PitchShifter pitchShifter[2][harmonies];

    juce::dsp::ProcessSpec spec;

    int previousPitchValue { 0 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DysomniPitchShifter)
};
