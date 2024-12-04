#pragma once
#include "JuceHeader.h"

class RingBufferForST
{
public:

    RingBufferForST() {}
    ~RingBufferForST() {}

    void initialise (int numChannels, int numSamples)
    {
        readPos.resize ((size_t) numChannels);
        writePos.resize ((size_t) numChannels);

        for (size_t i = 0; i < readPos.size(); i++)
        {
            readPos[i] = 0.0;
            writePos[i] = 0.0;
        }

        buffer.setSize ((int) numChannels, numSamples);
        pointerBuffer.resize ((size_t) (numChannels * numSamples));
    }

    void pushSample (float sample, int channel)
    {
        buffer.setSample (channel, writePos[(size_t) channel], sample);

        if (++writePos[(size_t) channel] >= buffer.getNumSamples())
        {
            writePos[(size_t) channel] = 0;
        }
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

        return writePos[channel] + buffer.getNumSamples() - readPos[channel];
    }

    const float* readPointerArray (int reqSamples)
    {
        for (size_t samplePos = 0; samplePos < (size_t) reqSamples; samplePos++)
        {
            for (size_t channel = 0; channel < (size_t) buffer.getNumChannels(); channel++)
            {
                pointerBuffer[samplePos * 2 + channel] = popSample (channel);
            }
        }

        return pointerBuffer.data();
    }

    void writePointerArray (float* ptrBegin, int writeNum)
    {
        for (size_t i = 0; i < (size_t) writeNum * 2; i++)
        {
            pointerBuffer[i] = *ptrBegin;
            ptrBegin++;
        }
    }

    void copyToBuffer (int numSamples)
    {
        for (size_t channel = 0; channel < (size_t) buffer.getNumChannels(); channel++)
        {
            for (size_t sample = 0; sample < (size_t) numSamples; sample++)
            {
                pushSample (pointerBuffer[sample * 2 + channel], (int) channel);
            }
        }
    }

private:

    juce::AudioBuffer<float> buffer; // , pointerBuffer;
    std::vector<float> pointerBuffer;
    std::vector<int> readPos, writePos;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RingBufferForST)
};
