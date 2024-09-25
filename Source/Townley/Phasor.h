#pragma once
#include <JuceHeader.h>

class Phasor
{
public:

    double getSample()
    {
        double ret = phase / PI_z_2;
        phase = std::fmod (phase + phase_inc, TAU); //

        return ret;
    }
    void setSampleRate (double v)
    {
        sampleRate = v;
        calculateIncrement();
    }
    void setFrequency (double v)
    {
        frequency = v;
        calculateIncrement();
    }
    void setPhase (double v)
    {
        phase = v;
        calculateIncrement();
    }
    void Reset() { phase = 0.0; }

    double getPhase_inc() { return phase_inc; }

protected:

    double PI = juce::MathConstants<float>::pi;

    void calculateIncrement() { phase_inc = TAU * frequency / sampleRate; }
    double sampleRate = 44100.0;
    double frequency = 1.0;
    double phase = 0.0;
    double phase_inc = 0.0;
    const double TAU = 2 * PI;
    const double PI_z_2 = PI / 2;
};
