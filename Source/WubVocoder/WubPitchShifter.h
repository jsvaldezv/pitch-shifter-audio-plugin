#pragma once
#include "PitchShifter.h"

class WubPitchShifter
{
public:

    WubPitchShifter() = default;
    ~WubPitchShifter() = default;

    void prepare (juce::dsp::ProcessSpec&);

    void process (juce::AudioBuffer<float>&);

    void setSemitones (int semitones);

private:

    std::vector<std::unique_ptr<WubVocoderPitchShifter>> wubVocoderPitchShifter; // https://github.com/professorwub/pitchshifter

    float wubShifterHopSize { 0.0f };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WubPitchShifter)
};