#pragma once
#include <JuceHeader.h>

class RingBufferMine
{
public:

    RingBufferMine() {}
    ~RingBufferMine() {}

    void initialise (int numChannels, int numSamples)
    {
        readPos.resize ((size_t) numChannels);
        writePos.resize ((size_t) numChannels);

        for (size_t i = 0; i < readPos.size(); i++)
        {
            readPos[i] = 0.0;
            writePos[i] = 0.0;
        }

        buffer.setSize (numChannels, numSamples);
        pointerBuffer.setSize (numChannels, numSamples);
    }

    void pushSample (float sample, size_t channel)
    {
        buffer.setSample ((int) channel, writePos[(size_t) channel], sample);

        if (++writePos[channel] >= buffer.getNumSamples())
            writePos[channel] = 0;
    }

    float popSample (size_t channel)
    {
        auto sample = buffer.getSample ((int) channel, readPos[channel]);

        if (++readPos[channel] >= buffer.getNumSamples())
            readPos[channel] = 0;

        return sample;
    }

    int getAvailableSampleNum (size_t channel)
    {
        if (readPos[channel] <= writePos[channel])
            return writePos[channel] - readPos[channel];

        return (buffer.getNumSamples() - readPos[channel]) + writePos[channel];
    }

    const float* const* readPointerArray (int reqSamples)
    {
        jassert (reqSamples <= buffer.getNumSamples()); // Asegurar que no se lea fuera de límites

        for (int samplePos = 0; samplePos < reqSamples; samplePos++)
        {
            for (int channel = 0; channel < buffer.getNumChannels(); channel++)
            {
                pointerBuffer.setSample (channel, samplePos, popSample ((size_t) channel));
            }
        }

        return pointerBuffer.getArrayOfReadPointers();
    }

    float* const* writePointerArray()
    {
        return pointerBuffer.getArrayOfWritePointers();
    }

    void copyToBuffer (int numSamples)
    {
        for (int channel = 0; channel < buffer.getNumChannels(); channel++)
        {
            for (int sample = 0; sample < numSamples; sample++)
            {
                pushSample (pointerBuffer.getSample (channel, sample), (size_t) channel);
            }
        }
    }

private:

    juce::AudioBuffer<float> buffer, pointerBuffer;
    std::vector<int> readPos, writePos;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RingBufferMine)
};
