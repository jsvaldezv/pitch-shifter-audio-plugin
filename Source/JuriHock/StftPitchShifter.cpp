#include "StftPitchShifter.h"

void StftPitchShifter::prepare (juce::dsp::ProcessSpec& sp)
{
    std::lock_guard lock (mutex);

    spec = sp;

    try
    {
        currentSemitones = -12;
        resetCore();
    }

    catch (const std::exception& exception)
    {
        juce::ignoreUnused (exception);
        DBG (exception.what());
    }
}

void StftPitchShifter::resetCore()
{
    const bool lowlatency = true;

    const double samplerate = spec.sampleRate;
    const int blocksize = lowlatency ? 512 : 2048;
    const int dftsize = getDftsize (blocksize, 1024); // "512", "1024", "2048", "4096", "8192"
    const int overlap = getOverlap (blocksize, 4); // "4", "8", "16", "32", "64"

    DBG ("Reset core (dftsize %d, overlap %d)" << dftsize << overlap);

    if (lowlatency)
        core = std::make_unique<InstantCore> (samplerate, blocksize, dftsize, overlap);
    else
        core = std::make_unique<DelayedCore> (samplerate, blocksize, dftsize, overlap);

    core->normalize (false);
    core->quefrency (0.0);
    core->timbre (1.0);
    updateSemitones();

    latency = core->latency();
    latencyChanged = true;

    DBG ("Latency %d (%d ms)" << latency << static_cast<int> (1e+3 * latency / samplerate));
}

void StftPitchShifter::process (juce::AudioBuffer<float>& buffer)
{
    std::lock_guard lock (mutex);

    latencyChanged = false;

    const int numSamples = buffer.getNumSamples();

    const auto process_mono_input = [&]()
    {
        auto input = std::span<const float> (buffer.getReadPointer (0), static_cast<size_t> (numSamples));
        auto output = std::span<float> (buffer.getWritePointer (0), static_cast<size_t> (numSamples));

        core->wet (input, output);
    };

    const auto process_stereo_output = [&] (const std::string& error = "")
    {
        if (! error.empty())
            DBG ("Copy input to output (%s)" << error.c_str());

        for (int channel = 1; channel < buffer.getNumChannels(); ++channel)
            buffer.copyFrom (channel, 0, buffer, 0, 0, numSamples);
    };

    TIC();

    if (! core)
        process_stereo_output ("core is not initialized");

    else if (! core->compatible (numSamples))
    {
        //State oldstate = state.value();
        //State newstate = oldstate;
        //newstate.blocksize.min = numSamples;

        //DBG ("Change blocksize from %d to %d" << oldstate.blocksize.min << newstate.blocksize.min);

        try
        {
            resetCore();
            process_mono_input();
            process_stereo_output();
        }

        catch (const std::exception& exception)
        {
            process_stereo_output (exception.what());
        }
    }
    else
    {
        try
        {
            process_mono_input();
            process_stereo_output();
        }

        catch (const std::exception& exception)
        {
            process_stereo_output (exception.what());
        }
    }

    TOC();
}

void StftPitchShifter::setSemitones (int semitones)
{
    std::lock_guard lock (mutex);

    if (core)
        currentSemitones = semitones;

    updateSemitones();
}

void StftPitchShifter::updateSemitones()
{
    std::set<double> factors;

    const int stages = 1;

    for (int i = 0; i < std::min (stages, 5); ++i)
    {
        const double factor = std::pow (2.0, (double) currentSemitones / 12.0);
        DBG (factor);
        factors.insert (factor);
    }

    std::vector<double> semitonesValue (factors.begin(), factors.end());

    core->pitch (semitonesValue);
}
