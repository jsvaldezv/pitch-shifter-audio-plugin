#pragma once
#include <JuceHeader.h>
#include <cassert>
#include <juce_core/juce_core.h>
#include <memory>
//#define DEBUGLOG

template <typename ElementType = float>
struct BlockCircularBuffer final
{
    BlockCircularBuffer() = default;
    BlockCircularBuffer (long newSize)
    {
        setSize (newSize, true);
    }

    void setReadHopSize (int hopSize)
    {
        readHopSize = hopSize;
    }

    auto getReadHopSize() const
    {
        return readHopSize;
    }

    void setWriteHopSize (int hopSize)
    {
        writeHopSize = hopSize;
    }

    auto getWriteHopSize() const
    {
        return writeHopSize;
    }

    auto getReadIndex() const
    {
        return readIndex;
    }

    auto getWriteIndex() const
    {
        return writeIndex;
    }

    void setSize (long newSize, bool shouldClear = false)
    {
        if (newSize == length)
        {
            if (shouldClear)
                reset();

            return;
        }

        block.allocate (newSize, shouldClear);
        length = newSize;
        writeIndex = readIndex = 0;
    }

    void setEnableLogging (const char* const bufferName, bool /*enabled*/)
    {
        name = bufferName;
    }

    void reset()
    {
        block.clear (length);
        writeIndex = readIndex = 0;
    }

    void read (ElementType* const destBuffer, const long destLength)
    {
        const auto firstReadAmount = readIndex + destLength >= length ? length - readIndex : destLength;

        assert (destLength <= length);
        assert (firstReadAmount <= destLength);

        const auto internalBuffer = block.getData();
        assert (internalBuffer != destBuffer);

        memcpy (destBuffer, internalBuffer + readIndex, sizeof (ElementType) * (juce::uint32) firstReadAmount);

        if (firstReadAmount < destLength)
        {
            memcpy (destBuffer + firstReadAmount, internalBuffer, sizeof (ElementType) * (static_cast<unsigned long long> (destLength) - (juce::uint32) firstReadAmount));
        }

        readIndex += readHopSize != 0 ? readHopSize : destLength;
        readIndex %= length;
    }

    void write (const ElementType* sourceBuffer, const long sourceLength)
    {
        const auto firstWriteAmount = writeIndex + sourceLength >= length ? length - writeIndex : sourceLength;

        auto internalBuffer = block.getData();
        assert (internalBuffer != sourceBuffer);
        memcpy (internalBuffer + writeIndex, sourceBuffer, sizeof (ElementType) * (juce::uint32) firstWriteAmount);

        if (firstWriteAmount < sourceLength)
        {
            memcpy (internalBuffer, sourceBuffer + firstWriteAmount, sizeof (ElementType) * (static_cast<unsigned long long> (sourceLength) - (juce::uint32) firstWriteAmount));
        }

        writeIndex += writeHopSize != 0 ? writeHopSize : sourceLength;
        writeIndex %= length;

        latestDataIndex = writeIndex + sourceLength % length;
    }

    void overlapWrite (ElementType* const sourceBuffer, const long sourceLength)
    {
        const int writeIndexDifference = (int) (getDifferenceBetweenIndexes ((int) writeIndex, (int) latestDataIndex, (int) length));
        const int overlapSampleCount = (int) (sourceLength - writeHopSize);
        const int overlapAmount = std::min (writeIndexDifference, overlapSampleCount);

        //DBG("writeIndexDifference: " << writeIndexDifference << ", overlapSampleCount: " << overlapSampleCount);
        auto tempWriteIndex = writeIndex;
        int firstWriteAmount = (int) (writeIndex + overlapAmount > length ? length - writeIndex : overlapAmount);
        //DBG("firstWriteAmout: " << firstWriteAmount << "\n");

        auto* internalBuffer = block.getData();

        juce::FloatVectorOperations::add (internalBuffer + writeIndex, sourceBuffer, firstWriteAmount);

        if (firstWriteAmount < overlapAmount)
        {
            juce::FloatVectorOperations::add (internalBuffer, sourceBuffer + firstWriteAmount, overlapAmount - firstWriteAmount);
        }

        tempWriteIndex += overlapAmount;
        tempWriteIndex %= length;

        const auto remainingElements = sourceLength - overlapAmount;
        firstWriteAmount = tempWriteIndex + remainingElements > length ? (int) (length - tempWriteIndex) : (int) remainingElements;

        memcpy (internalBuffer + tempWriteIndex, sourceBuffer + overlapAmount, sizeof (ElementType) * (juce::uint32) firstWriteAmount);

        if (firstWriteAmount < remainingElements)
        {
            memcpy (internalBuffer, sourceBuffer + overlapAmount + firstWriteAmount, sizeof (ElementType) * ((juce::uint32) remainingElements - static_cast<unsigned long long> (firstWriteAmount)));
        }

        latestDataIndex = (writeIndex + sourceLength) % length;
        writeIndex += writeHopSize;
        writeIndex %= length;
    }

private:

    int getDifferenceBetweenIndexes (int index1, int index2, int bufferLength)
    {
        return (index1 <= index2) ? index2 - index1 : bufferLength - index1 + index2;
    }

private:

    juce::HeapBlock<ElementType> block;
    long writeIndex = 0;
    long readIndex = 0;
    long length = 0;
    long latestDataIndex = 0;
    int writeHopSize = 0;
    int readHopSize = 0;

#ifdef DEBUG
    const char* name = "";
#endif
};
