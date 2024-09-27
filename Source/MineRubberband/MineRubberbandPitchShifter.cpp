#include "MineRubberbandPitchShifter.h"

void MineRubberbandPitchShifter::prepare (juce::dsp::ProcessSpec& sp)
{
    spec = sp;

    miPitch = std::make_unique<RubberBand::RubberBandStretcher> (spec.sampleRate,
                                                                 spec.numChannels,
                                                                 RubberBand::RubberBandStretcher::Option::OptionProcessRealTime,
                                                                 1.0,
                                                                 1.0);

    miPitch->reset();
    miPitch->setPitchOption (RubberBand::RubberBandStretcher::Option::OptionPitchHighConsistency);
    miPitch->setFormantOption (RubberBand::RubberBandStretcher::Option::OptionFormantPreserved);

    pitchBuffer.setSize ((int) spec.numChannels, (int) spec.maximumBlockSize);
    pitchBuffer.clear();
}

void MineRubberbandPitchShifter::process (juce::AudioBuffer<float>& buffer)
{
    pitchBuffer.makeCopyOf (buffer);

    miPitch->process (pitchBuffer.getArrayOfReadPointers(), (size_t) buffer.getNumSamples(), false);
    miPitch->retrieve (buffer.getArrayOfWritePointers(), (size_t) buffer.getNumSamples());
}

void MineRubberbandPitchShifter::setSemitones (int semitones)
{
    miPitch->setPitchScale (std::powf (2.0f, ((float) semitones / 12.0f)));
}
