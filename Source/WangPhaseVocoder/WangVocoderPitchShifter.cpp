#include "WangVocoderPitchShifter.h"

void WangVocoderPitchShifter::prepare (juce::dsp::ProcessSpec& sp)
{
    spec = sp;
}

void WangVocoderPitchShifter::process (juce::AudioBuffer<float>& buffer)
{
    for (int channel = 0; channel < buffer.getNumChannels(); channel++)
        pitchShifter[channel].process (buffer.getWritePointer (channel), buffer.getNumSamples());
}

void WangVocoderPitchShifter::setSemitones (int semitones)
{
    for (size_t channel = 0; channel < spec.numChannels; channel++)
        pitchShifter[channel].setPitchRatio (powf (2.0f, (float) semitones / 12.0f));
}
