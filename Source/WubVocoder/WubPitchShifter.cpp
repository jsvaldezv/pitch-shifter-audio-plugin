#include "WubPitchShifter.h"

void WubPitchShifter::prepare (juce::dsp::ProcessSpec& spec)
{
    int minWindowLength = (int) (16 * spec.maximumBlockSize);
    int order = 0;
    int windowLength = 1;

    while (windowLength < minWindowLength)
    {
        order++;
        windowLength *= 2;
    }

    wubVocoderPitchShifter.resize ((size_t) spec.numChannels);

    for (size_t ch = 0; ch < (size_t) spec.numChannels; ch++)
    {
        wubVocoderPitchShifter[ch] = std::unique_ptr<WubVocoderPitchShifter> (new WubVocoderPitchShifter);
        wubVocoderPitchShifter[ch]->init (order);
    }
}

void WubPitchShifter::process (juce::AudioBuffer<float>& buffer)
{
    for (size_t ch = 0; ch < (size_t) buffer.getNumChannels(); ch++)
        wubVocoderPitchShifter[ch]->step (buffer.getWritePointer ((int) ch), buffer.getNumSamples(), wubShifterHopSize);
}

void WubPitchShifter::setSemitones (int semitones)
{
    wubShifterHopSize = std::pow (2.0f, semitones / 12.0f);
}
