#include "DysomniPitchShifter.h"

void DysomniPitchShifter::prepare (juce::dsp::ProcessSpec& sp)
{
    spec = sp;

    for (size_t ch = 0; ch < spec.numChannels; ch++)
        pitchShifter[ch].setFs ((float) spec.sampleRate);
}

void DysomniPitchShifter::process (juce::AudioBuffer<float>& buffer)
{
    for (int channel = 0; channel < buffer.getNumChannels(); channel++)
    {
        for (int i = 0; i < buffer.getNumSamples(); i++)
        {
            float sample = buffer.getSample (channel, i);
            float out = pitchShifter[channel].processSample (sample);
            buffer.setSample (channel, i, out);
        }
    }
}

void DysomniPitchShifter::setSemitones (int semitones)
{
    for (size_t channel = 0; channel < spec.numChannels; channel++)
        pitchShifter[channel].setPitch (semitones);
}
