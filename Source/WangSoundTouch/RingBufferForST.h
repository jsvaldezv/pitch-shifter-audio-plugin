#pragma once
#include "JuceHeader.h"

class RingBufferForST
{
public:

    RingBufferForST() {}
    ~RingBufferForST() {}

    void initialise (int numChannels, int numSamples)
    {
        readPos.resize (numChannels);
        writePos.resize (numChannels);

        for (int i = 0; i < readPos.size(); i++)
        {
            readPos[i] = 0.0;
            writePos[i] = 0.0;
        }

        buffer.setSize (numChannels, numSamples);
        pointerBuffer.resize (numChannels * numSamples);
    }

    void pushSample (float sample, int channel)
    {
        buffer.setSample (channel, writePos[channel], sample);

        if (++writePos[channel] >= buffer.getNumSamples())
        {
            writePos[channel] = 0;
        }
    }

    float popSample (int channel)
    {
        auto sample = buffer.getSample (channel, readPos[channel]);

        if (++readPos[channel] >= buffer.getNumSamples())
        {
            readPos[channel] = 0;
        }
        return sample;
    }

    int getAvailableSampleNum (int channel)
    {
        if (readPos[channel] <= writePos[channel])
        {
            return writePos[channel] - readPos[channel];
        }
        else
        {
            return writePos[channel] + buffer.getNumSamples() - readPos[channel];
        }
    }

    const float* readPointerArray (int reqSamples)
    {
        for (int samplePos = 0; samplePos < reqSamples; samplePos++)
        {
            for (int channel = 0; channel < buffer.getNumChannels(); channel++)
            {
                pointerBuffer[samplePos * 2 + channel] = popSample (channel);
                // pointerBuffer.setSample(channel, samplePos, popSample(channel));
            }
        }
        return pointerBuffer.data(); // pointerBuffer.getArrayOfReadPointers();
    }

    void writePointerArray (float* ptrBegin, int writeNum)
    {
        for (int i = 0; i < writeNum * 2; i++)
        {
            pointerBuffer[i] = *ptrBegin;
            ptrBegin++;
        }
    }

    //return pointerBuffer.data();

    void copyToBuffer (int numSamples)
    {
        for (int channel = 0; channel < buffer.getNumChannels(); channel++)
        {
            for (int sample = 0; sample < numSamples; sample++)
            {
                pushSample (pointerBuffer[sample * 2 + channel], channel);
                // pushSample(pointerBuffer.getSample(channel, sample), channel);
            }
        }
    }

private:

    juce::AudioBuffer<float> buffer; // , pointerBuffer;
    std::vector<float> pointerBuffer;
    std::vector<int> readPos, writePos;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RingBufferForST)
};
