#pragma once

#include <JuceHeader.h>
#include <complex>
#include <memory>
#include <vector>

typedef std::complex<float> Cfloat;

class STFT
{
public:

    STFT() {}
    ~STFT() {}

    void init (int);
    void step (float*, int, Cfloat*);

private:

    int windowlen;

    std::vector<float> sampleframe;
    std::vector<float> hannwindow;
    std::vector<Cfloat> fftinput;

    std::unique_ptr<dsp::FFT> fft;
};