#pragma once

#include "istft.h"
#include "stft.h"

inline void PVresample (float* x, int Nx, float* y, int Ny)
{
    float ratio = float (Nx - 1) / float (Ny - 1);
    y[0] = x[0];
    y[Ny - 1] = x[Nx - 1];
    for (int n = 1; n < Ny - 1; n++)
    {
        float xn = ratio * float (n);
        int n1 = int (floor (xn));
        y[n] = x[n1] + (xn - floor (xn)) * (x[n1 + 1] - x[n1]);
    }
}

class WubVocoderPitchShifter
{
public:

    WubVocoderPitchShifter();
    ~WubVocoderPitchShifter() {}

    void init (int);
    void step (float*, int, float);

private:

    STFT stft;
    ISTFT istft;

    int windowlength;

    std::vector<Cfloat> fftdata;
    std::vector<float> fftmag;
    std::vector<float> fftphase;
    std::vector<float> oldphase;
    std::vector<float> modphase;
    std::vector<float> deltaphase;
    bool firsttime;
};
