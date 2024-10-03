#pragma once
#include "RingBufferMine.h"
#include "RubberBandStretcher.h"
#include <JuceHeader.h>

class WangRubberBandPitchShifter
{
public:

    WangRubberBandPitchShifter() {}
    ~WangRubberBandPitchShifter() {}

    void prepare (juce::dsp::ProcessSpec& spec, bool dryCompensationDelay = false, bool minLatency = false)
    {
        rubberband = std::make_unique<RubberBand::RubberBandStretcher> (spec.sampleRate, spec.numChannels, RubberBand::RubberBandStretcher::Option::OptionProcessRealTime + RubberBand::RubberBandStretcher::Option::OptionPitchHighConsistency, 1.0, 1.0);

        initLatency = (int) rubberband->getLatency();
        maxSamples = 256;

        input.initialise ((int) spec.numChannels, (int) spec.sampleRate);
        output.initialise ((int) spec.numChannels, (int) spec.sampleRate);

        if (dryCompensationDelay)
        {
            dryWet = std::make_unique<juce::dsp::DryWetMixer<float>> (spec.maximumBlockSize * 3.0 + initLatency);
            dryWet->prepare (spec);
            dryWet->setWetLatency ((float) (spec.maximumBlockSize * ((minLatency) ? 2.0 : 3.0) + initLatency));
        }
        else
        {
            dryWet = std::make_unique<juce::dsp::DryWetMixer<float>>();
            dryWet->prepare (spec);
        }

        timeSmoothing.reset (spec.sampleRate, 0.05);
        mixSmoothing.reset (spec.sampleRate, 0.5);
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
        
        reportLatency (getLatencyEstimationInSamples());
    }

    /** Pitch shift a juce::AudioBuffer<float>
     */
    void process (juce::AudioBuffer<float>& buffer)
    {
        dryWet->pushDrySamples (buffer);

        // Suavizado más largo para evitar cambios bruscos en el pitch
        pitchSmoothing.setTargetValue (std::powf (2.0f, pitchParam / 12.0f)); // Convert semitone to pitch scale value
        auto newPitch = pitchSmoothing.skip (buffer.getNumSamples());

        if (oldPitch != newPitch)
        {
            // Aplicar suavemente los cambios de pitch
            rubberband->setPitchScale (newPitch);
            oldPitch = newPitch;
        }

        for (int sample = 0; sample < buffer.getNumSamples(); sample++)
        {
            // Loop to push samples to input buffer.
            for (int channel = 0; channel < buffer.getNumChannels(); channel++)
            {
                input.pushSample (buffer.getSample (channel, sample), (size_t) channel);
                buffer.setSample (channel, sample, 0.0);

                if (channel == buffer.getNumChannels() - 1)
                {
                    auto reqSamples = rubberband->getSamplesRequired();

                    if (reqSamples <= (size_t) input.getAvailableSampleNum (0))
                    {
                        // Check to trigger rubberband to process when full enough.
                        auto readSpace = output.getAvailableSampleNum (0);

                        // Compress or stretch time when output ring buffer is too full or empty.
                        if (readSpace < smallestAcceptableSize)
                            timeSmoothing.setTargetValue (1.0);
                        else if (readSpace > largestAcceptableSize)
                            timeSmoothing.setTargetValue (1.0);
                        else
                            timeSmoothing.setTargetValue (1.0);

                        rubberband->setTimeRatio (timeSmoothing.skip ((int) reqSamples));
                        rubberband->process (input.readPointerArray ((int) reqSamples), reqSamples, false); // Process stored input samples.
                    }
                }
            }
        }

        auto availableSamples = rubberband->available();

        if (availableSamples > 0)
        {
            // If rubberband samples are available then copy to the output ring buffer.
            rubberband->retrieve (output.writePointerArray(), (size_t) availableSamples);
            output.copyToBuffer (availableSamples);
        }

        auto availableOutputSamples = output.getAvailableSampleNum (0);
        // DBG(availableOutputSamples);
        // Copy samples from output ring buffer to output buffer where available.
        for (int channel = 0; channel < buffer.getNumChannels(); channel++)
        {
            for (int sample = 0; sample < buffer.getNumSamples(); sample++)
            {
                if (output.getAvailableSampleNum ((size_t) channel) > 0)
                {
                    // DBG(availableOutputSamples);
                    //if (availableOutputSamples < buffer.getNumSamples())
                    //    DBG("available<numSamples!");// only begin at the beginning several
                    buffer.setSample (channel, ((availableOutputSamples >= buffer.getNumSamples()) ? sample : sample + buffer.getNumSamples() - availableOutputSamples), output.popSample ((size_t) channel));
                }
            }
        }

        // Ensure no phasing with mix occurs when pitch is set to +/-0 semitones.
        if (pitchParam == 0 && mixParam != 100.0)
            mixSmoothing.setTargetValue (0.0);
        else
            mixSmoothing.setTargetValue (mixParam / 100.0f);

        dryWet->setWetMixProportion (mixSmoothing.skip (buffer.getNumSamples()));
        dryWet->mixWetSamples (buffer);
    }

    /** Set the wet/dry mix as a % value.
     */
    void setMixPercentage (float newPercentage)
    {
        mixParam = newPercentage;
    }

    /** Set the pitch shift in semitones.
     */
    void setSemitones (float newShift)
    {
        pitchParam = newShift;
    }

    /** Get the % value of the wet/dry mix.
     */
    float getMixPercentage()
    {
        return mixParam;
    }

    /** Get the pitch shift in semitones.
     */
    float getSemitoneShift()
    {
        return pitchParam;
    }

    /** Get the estimated latency. This is an average guess of latency with no pitch shifting
    but can vary by a few buffers. Changing the pitch shift can cause less or more latency.
     */
    int getLatencyEstimationInSamples()
    {
        return (int) (maxSamples * 3.0 + initLatency);
    }
    
    std::function<void(int)> reportLatency;

private:

    std::unique_ptr<RubberBand::RubberBandStretcher> rubberband;
    RingBufferMine input, output;
    int maxSamples, initLatency, smallestAcceptableSize, largestAcceptableSize;
    float oldPitch, pitchParam, mixParam { 100.0f };
    std::unique_ptr<juce::dsp::DryWetMixer<float>> dryWet;
    juce::SmoothedValue<float> timeSmoothing, mixSmoothing, pitchSmoothing;

};
