#include "TownleyPitchShifter.h"

void TownleyPitchShifter::fillDelayBuffer (int channel, const int bufferLength, const int delayBufferLength, const float* bufferData, const float* delayBufferData)
{
    // Copy the data from main buffer to delay buffer
    // Check that our delayBufferLength is greater than bufferLength + mWritePosition. This will be the case most of the time, but not when the writePosition reaches a certain point
    
    if (delayBufferLength >= bufferLength + mWritePosition)
        mDelayBuffer.copyFromWithRamp (channel, mWritePosition, bufferData, bufferLength, 0.8, 0.8);
    else
    {
        // If delayBufferLength !> bufferLength + mWritePosition, we don't wanna copy a full bufffer's worth, only how many samples are left to fill in our delayBuffer
        const int delayBufferRemaining = delayBufferLength - mWritePosition;
        
        mDelayBuffer.copyFromWithRamp (channel, mWritePosition, bufferData, delayBufferRemaining, 0.8, 0.8);
        // We're at the end of our delayBuffer now. Keep copying, but our writePosition will be 0 now. Also, since we didn't copy the whole buffer last time, we need to make up for that.
        
        // Copy the leftover bit from the buffer, by adding it to bufferData. And the amount of samples we want to copy here is the leftover
        mDelayBuffer.copyFromWithRamp (channel, 0, bufferData + delayBufferRemaining, bufferLength - delayBufferRemaining, 0.8, 0.8);
    }
}
    
void TownleyPitchShifter::getFromDelayBuffer (juce::AudioBuffer<float>& buffer, int channel, const int bufferLength, const int delayBufferLength, const float* bufferData, const float* delayBufferData)
{
    // Interpolate
    
    float delaySamps = spec.sampleRate * (currentDelayMS) / 1000;
    
    // Create a read position. We want to be able to go back in time into our delay buffer and grab audio data.
    const int readPosition = static_cast<int> (delayBufferLength + mWritePosition - (delaySamps)) % delayBufferLength;
    
    // Now we want to add a signal from our delayBuffer to our regular buffer. We need to do some checks first.
    
    if (delayBufferLength > bufferLength + readPosition)
    {
        // If statement checks if there are enough values in the delay buffer.
        // Add from delayBuffer to buffer
        buffer.addFrom (channel, 0, delayBufferData + readPosition, bufferLength);
    }
    else
    {
        // how much space left in buffer?
        const int bufferRemaining = delayBufferLength - readPosition;
        buffer.addFrom (channel, 0, delayBufferData + readPosition, bufferRemaining);
        buffer.addFrom (channel, bufferRemaining, delayBufferData, bufferLength - bufferRemaining);
    }
}

void TownleyPitchShifter::prepare (juce::dsp::ProcessSpec& sp)
{
    spec = sp;
    
    const int delayBufferSize = 2.0 * (spec.sampleRate + spec.maximumBlockSize);
    
    mDelayBuffer.setSize (spec.numChannels, delayBufferSize);
    mDelayBuffer.clear();
}

void TownleyPitchShifter::process (juce::AudioBuffer<float>& buffer)
{
    const int bufferLength = buffer.getNumSamples();
    const int delayBufferLength = mDelayBuffer.getNumSamples();
    
    for (int channel = 0; channel < spec.numChannels; ++channel)
    {
        // Create read pointers into our buffers.
        const float* bufferData = buffer.getReadPointer (channel);
        const float* delayBufferData = mDelayBuffer.getReadPointer (channel);
        
        // Fill DBuffer does circular buffer stuff
        fillDelayBuffer(channel, bufferLength, delayBufferLength, bufferData, delayBufferData);
        
        // Call getFromDelayBuffer to copy data from delayBuffer to regular buffer (function unfinished)
        getFromDelayBuffer (buffer, channel, bufferLength, delayBufferLength, bufferData, delayBufferData);
    }
    
    // After copying a full buffer's samples into our delay buffer (512 samps), we want to INCREMENT THE WRITEPOSITION by the bufferLength
    mWritePosition += bufferLength;
    
    // Wrap around when we get to the end of our delayBuffer, so that when our writePosition exceeds the delayBufferLength, it starts writing from zero again.
    mWritePosition %= delayBufferLength;
}

void TownleyPitchShifter::setSemitones (int semitones)
{
    currentDelayMS = juce::jmap ((float) semitones, -12.0f, 12.0f, 0.0f, 1900.0f);
}
