#include "DysomniPitchShifter.h"

void DysomniPitchShifter::prepare (juce::dsp::ProcessSpec& sp)
{
    spec = sp;

    for (size_t ch = 0; ch < spec.numChannels; ch++)
        for (size_t h = 0; h < (size_t) harmonies; h++)
            pitchShifter[ch][h].setFs ((float) spec.sampleRate);
}

void DysomniPitchShifter::process (juce::AudioBuffer<float>& buffer)
{
    for (int channel = 0; channel < buffer.getNumChannels(); channel++)
    {
        for (int i = 0; i < buffer.getNumSamples(); i++)
        {
            float out = 0.0f;

            for (size_t h = 0; h < (size_t) harmonies; h++)
            {
                float sample = buffer.getSample (channel, i);

                sample = pitchShifter[channel][h].processSample (sample);

                out += sample;
            }

            buffer.setSample (channel, i, out * 0.2f);
        }
    }
}

void DysomniPitchShifter::setSemitones (int semitones)
{
    if (previousPitchValue != semitones)
    {
        for (size_t channel = 0; channel < spec.numChannels; channel++)
            for (size_t h = 0; h < (size_t) harmonies; h++)
                pitchShifter[channel][h].setPitch (semitones);

        previousPitchValue = semitones;
    }
}
