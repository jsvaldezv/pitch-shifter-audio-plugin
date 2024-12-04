#include "DelayedCore.h"

DelayedCore::DelayedCore (const double insamplerate, const int inblocksize, const int indftsize, const int inoverlap) : InstantCore (insamplerate, indftsize + indftsize, indftsize, inoverlap), host_block_size (inblocksize)
{
    const auto total_buffer_size = analysis_window_size + synthesis_window_size;

    buffer.input.resize (total_buffer_size);
    buffer.output.resize (total_buffer_size);

    samples = 0;
}

DelayedCore::~DelayedCore()
{
}

int DelayedCore::latency() const
{
    return 6 * dftsize - host_block_size;
}

bool DelayedCore::compatible (const int inblocksize) const
{
    return static_cast<size_t> (inblocksize) <= synthesis_window_size;
}

void DelayedCore::dry (const std::span<const float> input, const std::span<float> output)
{
    process (input, output, [&] (std::span<float> x, std::span<float> y)
    {
        InstantCore::dry (x, y);
    });
}

void DelayedCore::wet (const std::span<const float> input, const std::span<float> output)
{
    process (input, output, [&] (std::span<float> x, std::span<float> y)
    {
        InstantCore::wet (x, y);
    });
}

void DelayedCore::process (const std::span<const float> input, const std::span<float> output, std::function<void (std::span<float> x, std::span<float> y)> callback)
{
    const auto minsamples = input.size();
    const auto maxsamples = synthesis_window_size;

    // shift input buffer
    std::copy (
        buffer.input.begin() + (long) minsamples,
        buffer.input.end(),
        buffer.input.begin());

    // copy new input samples
    std::copy (
        input.begin(),
        input.end(),
        buffer.input.end() - (long) minsamples);

    // start processing as soon as enough samples are buffered
    if ((samples += minsamples) >= maxsamples)
    {
        const auto x = buffer.input.data() + buffer.input.size();
        const auto y = buffer.output.data() + buffer.output.size();

        callback (
            std::span (x - samples, maxsamples),
            std::span (y - samples, maxsamples));

        samples %= maxsamples;
    }

    // copy new output samples back
    std::copy (
        buffer.output.begin(),
        buffer.output.begin() + (long) minsamples,
        output.begin());

    // shift output buffer
    std::copy (
        buffer.output.begin() + (long) minsamples,
        buffer.output.end(),
        buffer.output.begin());
}
