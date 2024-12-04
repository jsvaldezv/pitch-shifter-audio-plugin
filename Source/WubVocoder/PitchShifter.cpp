#include "PitchShifter.h"

WubVocoderPitchShifter::WubVocoderPitchShifter()
{
    windowlength = 1;
    firsttime = true;
}

void WubVocoderPitchShifter::init (int order)
{
    stft.init (order);
    istft.init (order);
    windowlength = (int) powf (2.0f, order);
    fftdata.assign ((size_t) windowlength, 0.0f);
    fftmag.assign ((size_t) windowlength, 0.0f);
    fftphase.assign ((size_t) windowlength, 0.0f);
    oldphase.assign ((size_t) windowlength, 0.0f);
    modphase.assign ((size_t) windowlength, 0.0f);
    deltaphase.assign ((size_t) windowlength, 0.0f);
    firsttime = true;
}

void WubVocoderPitchShifter::step (float* samples, int numsamples, float hopratio)
{
    // Calculate STFT
    stft.step (samples, numsamples, fftdata.data());

    // Do strange and wonderful things in the frequency domain
    float pi = 3.14159f;

    for (size_t n = 0; n < (size_t) windowlength; n++)
    {
        // Separate magnitude and phase, saving previous phase
        oldphase[n] = fftphase[n];
        fftmag[n] = std::abs (fftdata[n]);
        fftphase[n] = std::arg (fftdata[n]);

        // Unwrap and modify phase
        deltaphase[n] = fftphase[n] - oldphase[n] - 2.0f * pi * float (numsamples) * float (n) / float (windowlength);
        deltaphase[n] -= 2.0f * pi * round (deltaphase[n] / (2.0f * pi));
        deltaphase[n] = (deltaphase[n] + 2.0f * pi * float (numsamples) * float (n) / float (windowlength)) * hopratio;

        if (firsttime)
            modphase[n] = fftphase[n];
        else
            modphase[n] += deltaphase[n];

        // Convert back to rectangular
        fftdata[n] = std::polar (fftmag[n], modphase[n]);
    }

    firsttime = false;

    // Calculate ISTFT using modified block size
    int synthlength = int (round (hopratio * float (numsamples)));
    std::vector<float> synthsamples;
    synthsamples.assign ((size_t) synthlength, 0.0f);
    istft.step (fftdata.data(), synthsamples.data(), synthlength);

    // Resample to original block size
    PVresample (synthsamples.data(), synthlength, samples, numsamples);
}
