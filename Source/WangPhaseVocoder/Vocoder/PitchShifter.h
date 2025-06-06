#pragma once
#include "PhaseVocoder.h"
#include <algorithm>
#include <vector>

class PitchShifterWangVocoder
{
public:

    PitchShifterWangVocoder()
#if USE_3rdPARTYPITCHSHIFT == false
        : synthPhaseIncrements ((size_t) phaseVocoder.getWindowSize(), 0),
          previousFramePhases ((size_t) phaseVocoder.getWindowSize(), 0)
#endif
    {
        setPitchRatio (1.f);
    }

    int getLatencyInSamples() const
    {
        return phaseVocoder.getLatencyInSamples();
    }

    void setPitchRatio (float newPitchRatio)
    {
        if (phaseVocoder.getPitchRatio() == newPitchRatio)
            return;

        const juce::SpinLock::ScopedLockType lock (phaseVocoder.getParamLock());
        phaseVocoder.setPitchRatio (newPitchRatio);
        phaseVocoder.setSynthesisHopSize ((int) (phaseVocoder.getWindowSize() / (float) phaseVocoder.getWindowOverlapCount()));
        phaseVocoder.setAnalysisHopSize ((int) round (phaseVocoder.getSynthesisHopSize() / phaseVocoder.getPitchRatio()));

#if USE_3rdPARTYPITCHSHIFT == false
        double accum = 0.0;
        auto windowFunction = phaseVocoder.getWindowFunction();

        for (int i = 0; i < phaseVocoder.getWindowSize(); ++i)
            accum += windowFunction[i] * (double) windowFunction[i];

        accum /= phaseVocoder.getSynthesisHopSize();
        phaseVocoder.setRescalingFactor ((float) accum);
        phaseVocoder.updateResampleBufferSize();
        phaseVocoder.setSynthesisHopSize (phaseVocoder.getAnalysisHopSize());

        memset (previousFramePhases.data(), 0, sizeof (FloatType) * (juce::uint32) phaseVocoder.getWindowSize());
        memset (synthPhaseIncrements.data(), 0, sizeof (FloatType) * (juce::uint32) phaseVocoder.getWindowSize());
#endif
    }

    void process (FloatType* const buffer, const int bufferSize)
    {
        phaseVocoder.process (buffer, bufferSize, [&] (FloatType* const inbuffer, const int inbufferSize)
        {
#if USE_3rdPARTYPITCHSHIFT == false
            for (size_t i = 0, x = 0; i < (size_t) inbufferSize - 1; i += 2, ++x)
            {
                const auto real = inbuffer[i];
                const auto imag = inbuffer[i + 1];
                const auto mag = sqrtf (real * real + imag * imag);
                const auto phase = atan2 (imag, real);

                const auto omega = juce::MathConstants<float>::twoPi * phaseVocoder.getAnalysisHopSize() * x / (float) phaseVocoder.getWindowSize();
                const auto deltaPhase = omega + PhaseVocoder::principalArgument (phase - (size_t) previousFramePhases[x] - omega);

                previousFramePhases[x] = phase;
                synthPhaseIncrements[x] = PhaseVocoder::principalArgument (synthPhaseIncrements[x] + (deltaPhase * phaseVocoder.getTimeStretchRatio()));

                inbuffer[i] = mag * std::cos (synthPhaseIncrements[x]);
                inbuffer[i + 1] = mag * std::sin (synthPhaseIncrements[x]);
            }
#endif
            setProcessFlag (true);
        });
    }

    bool getProcessFlag()
    {
        return phaseVocoder.getProcessFlag();
    }
    void setProcessFlag (bool flag)
    {
        phaseVocoder.setProcessFlag (flag);
    }

private:

    PhaseVocoder phaseVocoder;
#if USE_3rdPARTYPITCHSHIFT == false
    std::vector<FloatType> synthPhaseIncrements;
    std::vector<FloatType> previousFramePhases;
#endif
};
