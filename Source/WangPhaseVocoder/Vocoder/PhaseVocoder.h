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

    PhaseVocoder (int windowLength = 2048, int fftSize = 2048, Windows windowType = Windows::hann) : samplesTilNextProcess (windowLength),
                                                                                                     windowSize (windowLength),
                                                                                                     spectralBufferSize (windowLength * 2),
                                                                                                     analysisBuffer (windowLength),
                                                                                                     resampleBufferSize (windowLength),
                                                                                                     synthesisBuffer (windowLength * 3),

                                                                                                     windowFunction (windowLength),
                                                                                                     fft (std::make_unique<juce::dsp::FFT> (nearestPower2 (fftSize)))
    {
        windowOverlaps = getOverlapsRequiredForWindowType (windowType);
        analysisHopSize = windowLength / windowOverlaps;
        synthesisHopSize = windowLength / windowOverlaps;

        initialiseWindow (getWindowForEnum (windowType));

        spectralBufferSize = windowLength * (1 / MinPitchRatio) < spectralBufferSize ? (int) ceil (windowLength * (1 / MinPitchRatio)) : spectralBufferSize;

        spectralBuffer.resize (spectralBufferSize);

        std::fill (spectralBuffer.data(), spectralBuffer.data() + spectralBufferSize, 0.f);

        const auto maxResampleSize = (int) std::ceil (std::max (this->windowSize * MaxPitchRatio,
                                                                this->windowSize / MinPitchRatio));

        resampleBuffer.resize (maxResampleSize);
        std::fill (resampleBuffer.data(), resampleBuffer.data() + maxResampleSize, 0.f);
    }

    juce::SpinLock& getParamLock() { return paramLock; }

    int getWindowSize() const { return windowSize; }
    int getLatencyInSamples() const { return windowSize; }
    int getWindowOverlapCount() { return windowOverlaps; }

    float getPitchRatio() const { return pitchRatio; }

    void setPitchRatio (float newRatio)
    {
        pitchRatio = std::clamp (newRatio, PhaseVocoder::MinPitchRatio, PhaseVocoder::MaxPitchRatio);
    }

    float getTimeStretchRatio() const { return timeStretchRatio; }

    int getResampleBufferSize() const { return resampleBufferSize; }

    void updateResampleBufferSize()
    {
        resampleBufferSize = (int) std::ceil (windowSize * analysisHopSize / (float) synthesisHopSize);
        timeStretchRatio = synthesisHopSize / (float) analysisHopSize;
    }

    int getSynthesisHopSize() const { return synthesisHopSize; }

    void setSynthesisHopSize (int hopSize)
    {
        synthesisHopSize = hopSize;
    }

    int getAnalysisHopSize() const { return analysisHopSize; }

    void setAnalysisHopSize (int hopSize)
    {
        analysisHopSize = hopSize;
    }

    const FloatType* const getWindowFunction()
    {
        return windowFunction.data();
    }

    float getRescalingFactor() const { return rescalingFactor; }

    void setRescalingFactor (float factor)
    {
        rescalingFactor = factor;
    }

    void process (FloatType* const audioBuffer, const int audioBufferSize, std::function<void (FloatType* const, const int)> processCallback)
    {
        juce::ScopedNoDenormals noDenormals;
        const juce::SpinLock::ScopedLockType lock (paramLock);

        static int callbackCount = 0;

        /////////////////
        //DBG(" ");
        //DBG("Callback: " << ++callbackCount << ", SampleCount: " << incomingSampleCount <<
        //	", (+ incoming): " << audioBufferSize);
        //////////////////

        for (auto internalOffset = 0, internalBufferSize = 0;
             internalOffset < audioBufferSize;
             internalOffset += internalBufferSize)
        {
            const auto remainingIncomingSamples = audioBufferSize - internalOffset;
            internalBufferSize = incomingSampleCount + remainingIncomingSamples >= samplesTilNextProcess ? samplesTilNextProcess - incomingSampleCount : remainingIncomingSamples;
            /////////////////////////////////
            // DBG(samplesTilNextProcess);
            //DBG("Internal buffer: Offset: " << internalOffset << ", Size: " << internalBufferSize);
            /////////////////////////////////
            jassert (internalBufferSize <= audioBufferSize);

            const auto previousAnalysisWriteIndex = analysisBuffer.getReadIndex();
            analysisBuffer.write (audioBuffer + internalOffset, internalBufferSize);
            ///////////////////////////
            //DBG("Analysis Write Index: " << previousAnalysisWriteIndex << " -> " << analysisBuffer.getWriteIndex());
            ///////////////////////////
            incomingSampleCount += internalBufferSize;

            // Collected enough samples, do processing
            if (incomingSampleCount >= samplesTilNextProcess)
            {
                isProcessing = true;

                incomingSampleCount -= samplesTilNextProcess;
                ////////////////////////////////////////////////////////
                //DBG(" ");
                //DBG("Process: SampleCount: " << incomingSampleCount);
                ////////////////////////////////////////////////////////

                // After first processing, do another process every analysisHopSize samples
                samplesTilNextProcess = analysisHopSize;

                auto spectralBufferData = spectralBuffer.data();

                // jassert(spectralBufferSize > windowSize);
                analysisBuffer.setReadHopSize (analysisHopSize);
                analysisBuffer.read (spectralBufferData, windowSize);

                // spectralBuffer.resize(spectralBufferSize * 2);
                // std::fill(spectralBuffer.begin(), spectralBuffer.end(), 0);

                // Apply window to signal
                juce::FloatVectorOperations::multiply (spectralBufferData, windowFunction.data(), windowSize);

                // Rotate signal 180 degrees, move the first half to the back and back to the front
                std::rotate (spectralBufferData, spectralBufferData + (windowSize / 2), spectralBufferData + windowSize);

                // Perform FFT, process and inverse FFT
                fft->performRealOnlyForwardTransform (spectralBufferData);

                processCallback (spectralBufferData, spectralBufferSize);

                fft->performRealOnlyInverseTransform (spectralBufferData);

                // spectralBuffer.resize(spectralBufferSize);

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
                std::fill (audioBuffer + internalOffset, audioBuffer + internalOffset + internalBufferSize, 0.f);

                // DBG("Zeroed output: " << internalOffset << " -> " << internalBufferSize);
                continue;
            }

            const auto previousSynthesisReadIndex = synthesisBuffer.getReadIndex();
            synthesisBuffer.read (audioBuffer + internalOffset, internalBufferSize);
            //#endif

            //DBG("Synthesis Read Index: " << previousSynthesisReadIndex << " -> " << synthesisBuffer.getReadIndex());
        }
        // #if USE_3rdPARTYPITCHSHIFT==false
        // Rescale output
        juce::FloatVectorOperations::multiply (audioBuffer, 1.f / rescalingFactor, audioBufferSize);
        // #endif
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
        JuceWindow::fillWindowingTables (windowFunction.data(), windowSize, window, false);
    }

protected:
private:

    std::unique_ptr<juce::dsp::FFT> fft;

    // Buffers
    BlockCircularBuffer<FloatType> analysisBuffer;

    std::vector<FloatType> spectralBuffer;
    //#if USE_3rdPARTYPITCHSHIFT==false
    std::vector<FloatType> resampleBuffer;
    BlockCircularBuffer<FloatType> synthesisBuffer;
    //#endif
    // Misc state
    long incomingSampleCount = 0;
    int spectralBufferSize = 0;
    int samplesTilNextProcess = 0;
    bool isProcessing = false;

    juce::SpinLock paramLock;

    std::mutex flagLock;
    bool processDone { true };

    std::vector<FloatType> windowFunction;
    float rescalingFactor = 1.f;
    int analysisHopSize = 0;
    int synthesisHopSize = 0;
    int windowSize = 0;
    int resampleBufferSize = 0;
    int windowOverlaps = 0;

    int spectrumNum = 1024;

    float pitchRatio = 0.f;
    float timeStretchRatio = 1.f;
};
