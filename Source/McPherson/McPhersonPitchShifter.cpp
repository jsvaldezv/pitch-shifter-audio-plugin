#include "McPhersonPitchShifter.h"

void McPhersonPitchShifter::preparePitch (double inSampleRate, int inNumChannels)
{
    const double smoothTime = 1e-3;
    paramShift.reset (inSampleRate, smoothTime);

    updateFftSize (inNumChannels);
    updateHopSize();
    updateAnalysisWindow();
    updateWindowScaleFactor();

    needToResetPhases = true;
}

void McPhersonPitchShifter::processPitchShifting (juce::AudioSampleBuffer& inBuffer, float inSemitone)
{
    paramShift.setTargetValue (getScaleSemitone (inSemitone));

    int currentInputBufferWritePosition = 0;
    int currentOutputBufferWritePosition = 0;
    int currentOutputBufferReadPosition = 0;
    int currentSamplesSinceLastFFT = 0;

    float shift = paramShift.getNextValue();
    float ratio = roundf (shift * (float) hopSize) / (float) hopSize;
    int resampledLength = static_cast<int> (floorf ((float) fftSize / ratio));
    juce::HeapBlock<float> resampledOutput (resampledLength, true);
    juce::HeapBlock<float> synthesisWindow (resampledLength, true);
    updateWindow (synthesisWindow, resampledLength);

    for (int channel = 0; channel < inBuffer.getNumChannels(); channel++)
    {
        float* channelData = inBuffer.getWritePointer (channel);

        currentInputBufferWritePosition = inputBufferWritePosition;
        currentOutputBufferWritePosition = outputBufferWritePosition;
        currentOutputBufferReadPosition = outputBufferReadPosition;
        currentSamplesSinceLastFFT = samplesSinceLastFFT;

        for (int sample = 0; sample < inBuffer.getNumSamples(); sample++)
        {
            //======================================
            const float in = channelData[sample];
            channelData[sample] = outputBuffer.getSample (channel, currentOutputBufferReadPosition);

            //======================================
            outputBuffer.setSample (channel, currentOutputBufferReadPosition, 0.0f);
            if (++currentOutputBufferReadPosition >= outputBufferLength)
                currentOutputBufferReadPosition = 0;

            //======================================
            inputBuffer.setSample (channel, currentInputBufferWritePosition, in);
            if (++currentInputBufferWritePosition >= inputBufferLength)
                currentInputBufferWritePosition = 0;

            //======================================
            if (++currentSamplesSinceLastFFT >= hopSize)
            {
                currentSamplesSinceLastFFT = 0;

                //======================================
                int inputBufferIndex = currentInputBufferWritePosition;
                for (int index = 0; index < fftSize; ++index)
                {
                    fftTimeDomain[index].real (sqrtf (fftWindow[index]) * inputBuffer.getSample (channel, inputBufferIndex));
                    fftTimeDomain[index].imag (0.0f);

                    if (++inputBufferIndex >= inputBufferLength)
                        inputBufferIndex = 0;
                }

                fft->perform (fftTimeDomain, fftFrequencyDomain, false);

                if (paramShift.isSmoothing())
                    needToResetPhases = true;

                if (shift == paramShift.getTargetValue() && needToResetPhases)
                {
                    inputPhase.clear();
                    outputPhase.clear();
                    needToResetPhases = false;
                }

                for (int index = 0; index < fftSize; ++index)
                {
                    float magnitude = abs (fftFrequencyDomain[index]);
                    float phase = arg (fftFrequencyDomain[index]);

                    float phaseDeviation = phase - inputPhase.getSample (channel, index) - omega[index] * (float) hopSize;
                    float deltaPhi = omega[index] * hopSize + princArg (phaseDeviation);
                    float newPhase = princArg (outputPhase.getSample (channel, index) + deltaPhi * ratio);

                    inputPhase.setSample (channel, index, phase);
                    outputPhase.setSample (channel, index, newPhase);
                    fftFrequencyDomain[index] = std::polar (magnitude, newPhase);
                }

                fft->perform (fftFrequencyDomain, fftTimeDomain, true);

                for (int index = 0; index < resampledLength; ++index)
                {
                    float x = (float) index * (float) fftSize / (float) resampledLength;
                    int ix = (int) floorf (x);
                    float dx = x - (float) ix;

                    float sample1 = fftTimeDomain[ix].real();
                    float sample2 = fftTimeDomain[(ix + 1) % fftSize].real();
                    resampledOutput[index] = sample1 + dx * (sample2 - sample1);
                    resampledOutput[index] *= sqrtf (synthesisWindow[index]);
                }

                //======================================
                int outputBufferIndex = currentOutputBufferWritePosition;
                for (int index = 0; index < resampledLength; ++index)
                {
                    float out = outputBuffer.getSample (channel, outputBufferIndex);
                    out += resampledOutput[index] * windowScaleFactor;
                    outputBuffer.setSample (channel, outputBufferIndex, out);

                    if (++outputBufferIndex >= outputBufferLength)
                        outputBufferIndex = 0;
                }

                //======================================
                currentOutputBufferWritePosition += hopSize;
                if (currentOutputBufferWritePosition >= outputBufferLength)
                    currentOutputBufferWritePosition = 0;
            }
        }
    }

    inputBufferWritePosition = currentInputBufferWritePosition;
    outputBufferWritePosition = currentOutputBufferWritePosition;
    outputBufferReadPosition = currentOutputBufferReadPosition;
    samplesSinceLastFFT = currentSamplesSinceLastFFT;
}

void McPhersonPitchShifter::updateFftSize (int inNumChannels)
{
    fftSize = 512;
    fft = std::make_unique<juce::dsp::FFT> (static_cast<int> (log2 (fftSize)));

    inputBufferLength = fftSize;
    inputBufferWritePosition = 0;
    inputBuffer.clear();
    inputBuffer.setSize (inNumChannels, inputBufferLength);

    float maxRatio = getScaleSemitone (-12.0f);
    outputBufferLength = (int) floorf ((float) fftSize / maxRatio);
    outputBufferWritePosition = 0;
    outputBufferReadPosition = 0;
    outputBuffer.clear();
    outputBuffer.setSize (inNumChannels, outputBufferLength);

    fftWindow.realloc (fftSize);
    fftWindow.clear (fftSize);

    fftTimeDomain.realloc (fftSize);
    fftTimeDomain.clear (fftSize);

    fftFrequencyDomain.realloc (fftSize);
    fftFrequencyDomain.clear (fftSize);

    samplesSinceLastFFT = 0;

    omega.realloc (fftSize);
    for (int index = 0; index < fftSize; ++index)
        omega[index] = 2.0f * static_cast<float> (PI_VALUE) * index / (float) fftSize;

    inputPhase.clear();
    inputPhase.setSize (inNumChannels, outputBufferLength);

    outputPhase.clear();
    outputPhase.setSize (inNumChannels, outputBufferLength);
}

void McPhersonPitchShifter::updateHopSize()
{
    overlap = hopSize8;

    if (overlap != 0)
    {
        hopSize = fftSize / overlap;
        outputBufferWritePosition = hopSize % outputBufferLength;
    }
}

void McPhersonPitchShifter::updateAnalysisWindow()
{
    updateWindow (fftWindow, fftSize);
}

void McPhersonPitchShifter::updateWindow (const juce::HeapBlock<float>& window, const int windowLength)
{
    switch (windowTypeHann)
    {
        case windowTypeBartlett:
        {
            for (int sample = 0; sample < windowLength; ++sample)
                window[sample] = 1.0f - fabs (2.0f * (float) sample / (float) (windowLength - 1) - 1.0f);

            break;
        }

        case windowTypeHann:
        {
            for (int sample = 0; sample < windowLength; ++sample)
                window[sample] = 0.5f - 0.5f * cosf (2.0f * static_cast<float> (PI_VALUE) * (float) sample / (float) (windowLength - 1));

            break;
        }

        case windowTypeHamming:
        {
            for (int sample = 0; sample < windowLength; ++sample)
                window[sample] = 0.54f - 0.46f * cosf (2.0f * static_cast<float> (PI_VALUE) * (float) sample / (float) (windowLength - 1));

            break;
        }
    }
}

void McPhersonPitchShifter::updateWindowScaleFactor()
{
    float windowSum = 0.0f;
    for (int sample = 0; sample < fftSize; ++sample)
        windowSum += fftWindow[sample];

    windowScaleFactor = 0.0f;
    if (overlap != 0 && windowSum != 0.0f)
        windowScaleFactor = 1.0f / (float) overlap / windowSum * (float) fftSize;
}

float McPhersonPitchShifter::princArg (const float phase)
{
    if (phase >= 0.0f)
        return fmod (phase + static_cast<float> (PI_VALUE), 2.0f * static_cast<float> (PI_VALUE)) - static_cast<float> (PI_VALUE);
    else
        return fmod (phase + static_cast<float> (PI_VALUE), -2.0f * static_cast<float> (PI_VALUE)) + static_cast<float> (PI_VALUE);
}
