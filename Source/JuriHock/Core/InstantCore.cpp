#include "InstantCore.h"

InstantCore::InstantCore (const double insamplerate, const int inblocksize, const int indftsize, const int inoverlap) : Core (insamplerate, inblocksize, indftsize, inoverlap)
{
    const auto total_buffer_size = analysis_window_size + synthesis_window_size;

    buffer.input.resize (total_buffer_size);
    buffer.output.resize (total_buffer_size);
}

InstantCore::~InstantCore()
{
}

int InstantCore::latency() const
{
    return static_cast<int> (synthesis_window_size);
}

bool InstantCore::compatible (const int inblocksize) const
{
    return static_cast<size_t> (inblocksize) == synthesis_window_size;
}

void InstantCore::dry (const std::span<const float> input, const std::span<float> output)
{
    process (input, output, [] (std::span<double> x, std::span<double> y)
    {
        std::copy (x.begin(), x.end(), y.begin());
    });
}

void InstantCore::wet (const std::span<const float> input, const std::span<float> output)
{
    process (input, output, [&] (std::span<double> x, std::span<double> y)
    {
        stft_pitch_shift (x, y);
    });
}

void InstantCore::process (const std::span<const float> input, const std::span<float> output, std::function<void (std::span<double> x, std::span<double> y)> callback)
{
    // shift input buffer
    std::copy (
        buffer.input.begin() + (long) synthesis_window_size,
        buffer.input.end(),
        buffer.input.begin());

    // copy new input samples
    std::transform (
        input.begin(),
        input.end(),
        buffer.input.begin() + (long) analysis_window_size,
        transform<float, double>);

    // start processing
    callback (buffer.input, buffer.output);

    // copy new output samples back
    std::transform (
        (buffer.output.begin() + (long) analysis_window_size) - (long) synthesis_window_size,
        buffer.output.end() - (long) synthesis_window_size,
        output.begin(),
        transform<double, float>);

    // shift output buffer
    std::copy (
        buffer.output.begin() + (long) synthesis_window_size,
        buffer.output.end(),
        buffer.output.begin());

    // prepare for the next callback
    std::fill (
        buffer.output.begin() + (long) analysis_window_size,
        buffer.output.end(),
        0);
}
