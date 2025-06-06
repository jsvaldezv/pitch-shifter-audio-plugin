#include "Core.h"

Core::Core (const double insamplerate, const int inblocksize, const int indftsize, const int inoverlap) : samplerate (insamplerate), blocksize (inblocksize), dftsize (indftsize), overlap (inoverlap), analysis_window_size (static_cast<size_t> (indftsize + indftsize)), synthesis_window_size (static_cast<size_t> (inblocksize))
{
    const auto winsize = std::make_tuple (analysis_window_size, synthesis_window_size);
    const auto hopsize = synthesis_window_size / static_cast<size_t> (overlap);

    const auto fft = std::make_shared<FFT>();

    stft = std::make_unique<stftpitchshift::STFT<double>> (fft, winsize, hopsize);
    core = std::make_unique<stftpitchshift::StftPitchShiftCore<double>> (fft, winsize, hopsize, samplerate);
}

Core::~Core() {}

void Core::normalize (bool value)
{
    core->normalization (value);
}

void Core::quefrency (double value)
{
    core->quefrency (value);
}

void Core::timbre (double value)
{
    core->distortion (value);
}

void Core::pitch (std::vector<double> values)
{
    core->factors (values);
}

void Core::stft_pitch_shift (const std::span<const double> input, const std::span<double> output) const
{
    (*stft) (input, output, [&] (std::span<std::complex<double>> dft)
    {
        core->shiftpitch (dft);
    });
}
