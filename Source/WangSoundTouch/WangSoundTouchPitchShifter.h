#pragma once

#include "JuceHeader.h"
#include "RingBufferForST.h"
#include "SoundTouch/SoundTouch.h"

class WangSoundTouchPitchShifter
{
public:

    WangSoundTouchPitchShifter() : samplesPerBlock (512) {}
    ~WangSoundTouchPitchShifter() {}

    void prepare (juce::dsp::ProcessSpec& spec, bool dryCompensationDelay = false, bool minLatency = true)
    {
        st = std::make_unique<soundtouch::SoundTouch>();
        st->setSampleRate ((juce::uint32) spec.sampleRate);
        st->setChannels (spec.numChannels);
        // st->setSetting(SOUNDTOUCH_ALLOW_MMX, 1);
        st->setSetting (SETTING_USE_AA_FILTER, 1);
        st->setSetting (SETTING_AA_FILTER_LENGTH, 64);
        st->setSetting (SETTING_USE_QUICKSEEK, 0);
        st->setSetting (SETTING_SEQUENCE_MS, 60);
        st->setSetting (SETTING_SEEKWINDOW_MS, 25);
        //soundtouch::SAMPLETYPE
        maxSamples = 256;

        input.initialise ((int) spec.numChannels, (int) spec.sampleRate);
        output.initialise ((int) spec.numChannels, (int) spec.sampleRate);

        if (dryCompensationDelay)
        {
            dryWet = std::make_unique<juce::dsp::DryWetMixer<float>> (spec.maximumBlockSize * 3.0 + initLatency);
            dryWet->prepare (spec);
            dryWet->setWetLatency (spec.maximumBlockSize * ((minLatency) ? 2.0f : 3.0f) + initLatency);
        }
        else
        {
            dryWet = std::make_unique<juce::dsp::DryWetMixer<float>>();
            dryWet->prepare (spec);
        }

        timeSmoothing.reset (spec.sampleRate, 0.05);
        mixSmoothing.reset (spec.sampleRate, 0.3);
        pitchSmoothing.reset (spec.sampleRate, 0.1);

        if (minLatency)
        {
            smallestAcceptableSize = (int) (maxSamples * 1.0);
            largestAcceptableSize = (int) (maxSamples * 3.0);
        }
        else
        {
            smallestAcceptableSize = (int) (maxSamples * 2.0);
            largestAcceptableSize = (int) (maxSamples * 4.0);
        }

        setMixPercentage (100.0f);
    }

    /** Pitch shift a juce::AudioBuffer<float>
     */
    void process (juce::AudioBuffer<float>& buffer)
    {
        dryWet->pushDrySamples (buffer);

        pitchSmoothing.setTargetValue (powf (2.0, pitchParam / 12)); // Convert semitone value into pitch scale value.
        auto newPitch = pitchSmoothing.skip (buffer.getNumSamples());

        if (oldPitch != newPitch)
        {
            st->setPitch (newPitch);
        }

        for (int sample = 0; sample < buffer.getNumSamples(); sample++)
        {
            // Loop to push samples to input buffer.
            for (int channel = 0; channel < buffer.getNumChannels(); channel++)
            {
                input.pushSample (buffer.getSample (channel, sample), channel);
                buffer.setSample (channel, sample, 0.0);

                if (channel == buffer.getNumChannels() - 1)
                {
                    auto reqSamples = samplesPerBlock;

                    if (reqSamples <= input.getAvailableSampleNum (0))
                    {
                        // Check to trigger rubberband to process when full enough.
                        auto readSpace = output.getAvailableSampleNum (0);

                        if (readSpace < smallestAcceptableSize)
                        {
                            // Compress or stretch time when output ring buffer is too full or empty.
                            timeSmoothing.setTargetValue (1.0);
                        }
                        else if (readSpace > largestAcceptableSize)
                        {
                            // DBG("readSpace:" << readSpace);
                            timeSmoothing.setTargetValue (1.0);
                        }
                        else
                        {
                            timeSmoothing.setTargetValue (1.0);
                        }

                        st->putSamples (input.readPointerArray ((int) reqSamples), static_cast<unsigned int> (reqSamples));
                    }
                }
            }
        }

        auto availableSamples = static_cast<int> (st->numSamples());

        if (availableSamples > 0)
        {
            float* readSample = st->ptrBegin();
            output.writePointerArray (readSample, availableSamples);
            st->receiveSamples (static_cast<unsigned int> (availableSamples));
            output.copyToBuffer (availableSamples);
        }

        auto availableOutputSamples = output.getAvailableSampleNum (0);

        // DBG(availableOutputSamples);
        for (int channel = 0; channel < buffer.getNumChannels(); channel++)
        {
            for (int sample = 0; sample < buffer.getNumSamples(); sample++)
            {
                if (output.getAvailableSampleNum ((size_t) channel) > 0)
                {
                    buffer.setSample (channel, ((availableOutputSamples >= buffer.getNumSamples()) ? sample : sample + buffer.getNumSamples() - availableOutputSamples), output.popSample ((size_t) channel));
                }
            }
        }

        if (pitchParam == 0 && mixParam != 100.0)
        {
            mixSmoothing.setTargetValue (0.0);
        }
        else
        {
            mixSmoothing.setTargetValue (mixParam / 100.0);
        }

        dryWet->setWetMixProportion ((float) (mixSmoothing.skip (buffer.getNumSamples())));
        dryWet->mixWetSamples (buffer);
    }

    void setMixPercentage (float newPercentage)
    {
        mixParam = newPercentage;
    }

    void setSemitones (float newShift)
    {
        pitchParam = newShift;
    }

    float getMixPercentage()
    {
        return mixParam;
    }

    float getSemitoneShift()
    {
        return pitchParam;
    }

    int getLatencyEstimationInSamples()
    {
        return (int) (maxSamples * 3.0 + initLatency);
    }

private:

    std::unique_ptr<soundtouch::SoundTouch> st;
    RingBufferForST input, output;
    juce::AudioBuffer<float> inputBuffer, outputBuffer;
    int maxSamples, initLatency, smallestAcceptableSize, largestAcceptableSize, samplesPerBlock;
    float oldPitch, pitchParam, mixParam { 100.0f };
    std::unique_ptr<juce::dsp::DryWetMixer<float>> dryWet;
    juce::SmoothedValue<double> timeSmoothing, mixSmoothing, pitchSmoothing;
};
