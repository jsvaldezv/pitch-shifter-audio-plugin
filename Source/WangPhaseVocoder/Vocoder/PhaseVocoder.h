#pragma once
#include "BlockCircularBuffer.h"
#include "Resample.h"
#include <algorithm>
#include <functional>
#include <juce_dsp/juce_dsp.h>

using FloatType = float;

class PhaseVocoder
{
public:

    static constexpr float MaxPitchRatio = 2.0f;
    static constexpr float MinPitchRatio = 0.5f;

    enum class Windows
    {
        hann,
        hamming,
        kaiser
    };

public:

    PhaseVocoder (int windowLength = 2048, int fftSize = 2048, Windows windowType = Windows::hann) : fft (std::make_unique<juce::dsp::FFT> (nearestPower2 (fftSize))),
                                                                                                     analysisBuffer (windowLength),
                                                                                                     synthesisBuffer (windowLength * 3),
                                                                                                     spectralBufferSize (windowLength * 2),
                                                                                                     samplesTilNextProcess (windowLength),
                                                                                                     windowFunction ((size_t) windowLength),
                                                                                                     windowSize (windowLength),
                                                                                                     resampleBufferSize (windowLength)
    {
        windowOverlaps = getOverlapsRequiredForWindowType (windowType);
        analysisHopSize = windowLength / windowOverlaps;
        synthesisHopSize = windowLength / windowOverlaps;

        initialiseWindow (getWindowForEnum (windowType));

        spectralBufferSize = windowLength * (1 / MinPitchRatio) < spectralBufferSize ? (int) ceil (windowLength * (1 / MinPitchRatio)) : spectralBufferSize;

        spectralBuffer.resize ((size_t) spectralBufferSize);

        std::fill (spectralBuffer.data(), spectralBuffer.data() + spectralBufferSize, 0.0f);

        const auto maxResampleSize = (int) std::ceil (std::max (this->windowSize * MaxPitchRatio,
                                                                this->windowSize / MinPitchRatio));

        resampleBuffer.resize ((size_t) maxResampleSize);
        std::fill (resampleBuffer.data(), resampleBuffer.data() + maxResampleSize, 0.0f);
    }

    juce::SpinLock& getParamLock()
    {
        return paramLock;
    }

    int getWindowSize() const
    {
        return windowSize;
    }

    int getLatencyInSamples() const
    {
        return windowSize;
    }

    int getWindowOverlapCount()
    {
        return windowOverlaps;
    }

    float getPitchRatio() const
    {
        return pitchRatio;
    }

    void setPitchRatio (float newRatio)
    {
        pitchRatio = std::clamp (newRatio, PhaseVocoder::MinPitchRatio, PhaseVocoder::MaxPitchRatio);
    }

    float getTimeStretchRatio() const
    {
        return timeStretchRatio;
    }

    int getResampleBufferSize() const
    {
        return resampleBufferSize;
    }

    void updateResampleBufferSize()
    {
        resampleBufferSize = (int) std::ceil (windowSize * analysisHopSize / (float) synthesisHopSize);
        timeStretchRatio = synthesisHopSize / (float) analysisHopSize;
    }

    int getSynthesisHopSize() const
    {
        return synthesisHopSize;
    }

    void setSynthesisHopSize (int hopSize)
    {
        synthesisHopSize = hopSize;
    }

    int getAnalysisHopSize() const
    {
        return analysisHopSize;
    }

    void setAnalysisHopSize (int hopSize)
    {
        analysisHopSize = hopSize;
    }

    const FloatType* const getWindowFunction()
    {
        return windowFunction.data();
    }

    float getRescalingFactor() const
    {
        return rescalingFactor;
    }

    void setRescalingFactor (float factor)
    {
        rescalingFactor = factor;
    }

    void process (FloatType* const audioBuffer, const int audioBufferSize, std::function<void (FloatType* const, const int)> processCallback)
    {
        juce::ScopedNoDenormals noDenormals;
        const juce::SpinLock::ScopedLockType lock (paramLock);

        for (auto internalOffset = 0, internalBufferSize = 0; internalOffset < audioBufferSize; internalOffset += internalBufferSize)
        {
            const auto remainingIncomingSamples = audioBufferSize - internalOffset;

            internalBufferSize = (int) (incomingSampleCount + remainingIncomingSamples >= samplesTilNextProcess ? samplesTilNextProcess - incomingSampleCount : remainingIncomingSamples);

            jassert (internalBufferSize <= audioBufferSize);

            analysisBuffer.write (audioBuffer + internalOffset, internalBufferSize);
            incomingSampleCount += internalBufferSize;

            // Collected enough samples, do processing
            if (incomingSampleCount >= samplesTilNextProcess)
            {
                isProcessing = true;

                incomingSampleCount -= samplesTilNextProcess;

                // After first processing, do another process every analysisHopSize samples
                samplesTilNextProcess = analysisHopSize;

                auto spectralBufferData = spectralBuffer.data();

                // jassert(spectralBufferSize > windowSize);
                analysisBuffer.setReadHopSize (analysisHopSize);
                analysisBuffer.read (spectralBufferData, windowSize);

                // Apply window to signal
                juce::FloatVectorOperations::multiply (spectralBufferData, windowFunction.data(), windowSize);

                // Rotate signal 180 degrees, move the first half to the back and back to the front
                std::rotate (spectralBufferData, spectralBufferData + (windowSize / 2), spectralBufferData + windowSize);

                // Perform FFT, process and inverse FFT
                fft->performRealOnlyForwardTransform (spectralBufferData);

                processCallback (spectralBufferData, spectralBufferSize);

                fft->performRealOnlyInverseTransform (spectralBufferData);

                // Undo signal back to original rotation
                std::rotate (spectralBufferData, spectralBufferData + (windowSize / 2), spectralBufferData + windowSize);

                // Apply window to signal
                juce::FloatVectorOperations::multiply (spectralBufferData, windowFunction.data(), windowSize);

                // Resample output grain to N * (hop size analysis / hop size synthesis)
                linearResample (spectralBufferData, windowSize, resampleBuffer.data(), resampleBufferSize);
                synthesisBuffer.setWriteHopSize (synthesisHopSize);
                synthesisBuffer.overlapWrite (resampleBuffer.data(), resampleBufferSize);

                setProcessFlag (true);
            }

            // Emit silence until we start producing output
            if (! isProcessing)
            {
                std::fill (audioBuffer + internalOffset, audioBuffer + internalOffset + internalBufferSize, 0.0f);
                continue;
            }

            synthesisBuffer.read (audioBuffer + internalOffset, internalBufferSize);
        }

        juce::FloatVectorOperations::multiply (audioBuffer, 1.0f / rescalingFactor, audioBufferSize);
    }

    // Principal argument - Unwrap a phase argument to between [-PI, PI]
    static float principalArgument (float arg)
    {
        return std::fmod (arg + juce::MathConstants<FloatType>::pi,
                          -juce::MathConstants<FloatType>::twoPi)
               + juce::MathConstants<FloatType>::pi;
    }

    static int nearestPower2 (int value)
    {
        return (int) log2 (juce::nextPowerOfTwo (value));
    }

    void setProcessFlag (bool flag)
    {
        std::lock_guard<std::mutex> lock (flagLock);
        processDone = flag;
    }

    bool getProcessFlag()
    {
        std::lock_guard<std::mutex> lock (flagLock);
        return processDone;
    }

private:

    using JuceWindow = typename juce::dsp::WindowingFunction<FloatType>;
    using JuceWindowTypes = typename juce::dsp::WindowingFunction<FloatType>::WindowingMethod;

    int getOverlapsRequiredForWindowType (Windows windowType) const
    {
        switch (windowType)
        {
            case Windows::hann:
            case Windows::hamming:
                return 4;

            case Windows::kaiser:
                return 8;

            default:
                return -1;
        }
    }

    JuceWindowTypes getWindowForEnum (Windows windowType)
    {
        switch (windowType)
        {
            case Windows::kaiser:
                return JuceWindow::kaiser;

            case Windows::hamming:
                return JuceWindow::hamming;

            case Windows::hann:
            default:
                return JuceWindow::hann;
        }
    }

    void initialiseWindow (JuceWindowTypes window)
    {
        JuceWindow::fillWindowingTables (windowFunction.data(), (size_t) windowSize, window, false);
    }

protected:
private:

    std::unique_ptr<juce::dsp::FFT> fft;

    // Buffers
    BlockCircularBuffer<FloatType> analysisBuffer;

    std::vector<FloatType> spectralBuffer;
    std::vector<FloatType> resampleBuffer;
    BlockCircularBuffer<FloatType> synthesisBuffer;

    // Misc state
    long incomingSampleCount = 0;
    int spectralBufferSize = 0;
    int samplesTilNextProcess = 0;
    bool isProcessing = false;

    juce::SpinLock paramLock;

    std::mutex flagLock;
    bool processDone { true };

    std::vector<FloatType> windowFunction;
    float rescalingFactor = 1.0f;
    int analysisHopSize = 0;
    int synthesisHopSize = 0;
    int windowSize = 0;
    int resampleBufferSize = 0;
    int windowOverlaps = 0;

    float pitchRatio = 0.0f;
    float timeStretchRatio = 1.0f;
};
