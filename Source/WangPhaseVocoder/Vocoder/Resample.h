#pragma once
#include <cmath>

static void linearResample (const float* const originalSignal, const int originalSize, float* const newSignal, const int newSignalSize)
{
    const auto lerp = [&] (double v0, double v1, double t)
    {
        return (1.f - t) * v0 + t * v1;
    };

    const auto scale = originalSize / (double) newSignalSize;
    double index = 0.f;

    for (int i = 0; i < newSignalSize; ++i)
    {
        const auto wholeIndex = (int) std::floor (index);
        const auto fractionIndex = index - wholeIndex;
        const auto sampleA = originalSignal[wholeIndex];
        const auto sampleB = originalSignal[wholeIndex + 1];
        newSignal[i] = (float) lerp (sampleA, sampleB, fractionIndex);
        index += scale;
    }
}
