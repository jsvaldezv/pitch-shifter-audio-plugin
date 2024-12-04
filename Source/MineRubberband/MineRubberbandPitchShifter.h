#pragma once
#include "RubberBandStretcher.h"
#include <JuceHeader.h>

class MineRubberbandPitchShifter
{
public:

    MineRubberbandPitchShifter() = default;
    ~MineRubberbandPitchShifter() = default;

    void prepare (juce::dsp::ProcessSpec&);

    void process (juce::AudioBuffer<float>&);

    void setSemitones (int semitones);

private:

    std::unique_ptr<RubberBand::RubberBandStretcher> miPitch;
    juce::AudioBuffer<float> pitchBuffer;

    juce::dsp::ProcessSpec spec;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MineRubberbandPitchShifter)
};
