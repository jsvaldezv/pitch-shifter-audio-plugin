#pragma once
#include <JuceHeader.h>

class McPhersonPitchShifter
{
public:

    McPhersonPitchShifter() = default;
    ~McPhersonPitchShifter() = default;

    void preparePitch (double inSampleRate, int inNumChannels);

    float getScaleSemitone (float inValue)
    {
        return powf (2.0f, inValue / 12.0f);
    }

    void processPitchShifting (juce::AudioSampleBuffer& inBuffer, float inSemitone);

    void updateFftSize (int inNumChannels);

    void updateHopSize();

    void updateAnalysisWindow();

    void updateWindow (const juce::HeapBlock<float>& window, const int windowLength);

    void updateWindowScaleFactor();

    float princArg (const float phase);

private:

    enum windowTypeIndex
    {
        windowTypeBartlett = 0,
        windowTypeHann,
        windowTypeHamming,
    };

    enum hopSizeIndex
    {
        hopSize2 = 0,
        hopSize4,
        hopSize8,
    };

    juce::CriticalSection lock;

    int fftSize;
    std::unique_ptr<juce::dsp::FFT> fft;

    int inputBufferLength;
    int inputBufferWritePosition;
    juce::AudioSampleBuffer inputBuffer;

    int outputBufferLength;
    int outputBufferWritePosition;
    int outputBufferReadPosition;
    juce::AudioSampleBuffer outputBuffer;

    juce::HeapBlock<float> fftWindow;
    juce::HeapBlock<juce::dsp::Complex<float>> fftTimeDomain;
    juce::HeapBlock<juce::dsp::Complex<float>> fftFrequencyDomain;

    int samplesSinceLastFFT;

    int overlap;
    int hopSize;
    float windowScaleFactor;

    juce::HeapBlock<float> omega;
    juce::AudioSampleBuffer inputPhase;
    juce::AudioSampleBuffer outputPhase;
    bool needToResetPhases;

    // Smoothed value for pitch
    juce::LinearSmoothedValue<float> paramShift { 0.0f };

    float PI_VALUE = juce::MathConstants<float>::pi;
};
