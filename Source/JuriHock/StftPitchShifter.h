#pragma once
#include "Chronometer.h"
#include "Core.h"
#include "Core/DelayedCore.h"
#include "Core/InstantCore.h"
#include <JuceHeader.h>
#include <span>

static int prev_power_of_two (const int x)
{
    jassert (x >= 0);
    const auto y = std::bit_floor (static_cast<unsigned int> (x));
    return static_cast<int> (y);
}

static int next_power_of_two (const int x)
{
    jassert (x >= 0);
    const auto y = std::bit_ceil (static_cast<unsigned int> (x));
    jassert (y <= static_cast<unsigned int> (std::numeric_limits<int>::max()));
    return static_cast<int> (y);
}

inline int getDftsize (int blocksize, int dftsize)
{
    dftsize = std::max (dftsize, 512);
    dftsize = next_power_of_two (dftsize);

    while ((dftsize * 2 < blocksize) && (dftsize < 65536))
    {
        dftsize = next_power_of_two (dftsize + 1);
    }

    return dftsize;
}

inline int getOverlap (int blocksize, int overlap)
{
    overlap = std::max (overlap, 1);
    overlap = prev_power_of_two (overlap);

    while ((overlap * 4 > blocksize) && (overlap > 1))
    {
        overlap = prev_power_of_two (overlap - 1);
    }

    return overlap;
}

class StftPitchShifter
{
public:

    StftPitchShifter() = default;
    ~StftPitchShifter() = default;

    void prepare (juce::dsp::ProcessSpec&);

    void process (juce::AudioBuffer<float>&);

    void setSemitones (int semitones);

    void updateSemitones();

    int getLatency() { return latency; }

private:

    struct State
    {
        double samplerate {};
        struct
        {
            int min, max;
        } blocksize {};
    };

    const State nostate;

    juce::dsp::ProcessSpec spec;

    int latency { 0 };

    int currentSemitones { 0 };

    std::mutex mutex;
    std::optional<State> state;
    std::unique_ptr<Core> core;

    void resetCore();
    void resetCore (const State& state);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StftPitchShifter)
};
