#pragma once

#include <JuceHeader.h>
#include <complex>
#include <memory>
#include <vector>

typedef std::complex<float> Cfloat;

class ISTFT
{
public:

    ISTFT() {}
    ~ISTFT() {}

    void init (int);
    void step (Cfloat*, float*, int);

private:

    int windowlen;

    std::vector<float> sampleframe;
    std::vector<float> hannwindow;
    std::vector<Cfloat> fftoutput;

    float hannsum;

    std::unique_ptr<dsp::FFT> fft;
};