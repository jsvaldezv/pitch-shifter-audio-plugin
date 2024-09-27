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

    PitchShifter pitchShifter[2];

    juce::dsp::ProcessSpec spec;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DysomniPitchShifter)
};
