#include "Shifter.h"

SHIFTER::SHIFTER()
{
    windowlength = 1;
    firsttime = true;
}

void SHIFTER::init (int order)
{
    stft.init (order);
    istft.init (order);
    windowlength = pow (2.0f, order);
    fftdata.assign (windowlength, 0.0f);
    fftmag.assign (windowlength, 0.0f);
    fftphase.assign (windowlength, 0.0f);
    oldphase.assign (windowlength, 0.0f);
    modphase.assign (windowlength, 0.0f);
    deltaphase.assign (windowlength, 0.0f);
    firsttime = true;
}

void SHIFTER::step (float* samples, int numsamples, float hopratio)
{
    // Calculate STFT
    stft.step (samples, numsamples, fftdata.data());
    
    // Do strange and wonderful things in the frequency domain
    float pi = 3.14159f;
    
    for (int n = 0; n < windowlength; n++)
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
    synthsamples.assign (synthlength, 0.0f);
    istft.step (fftdata.data(), synthsamples.data(), synthlength);
    
    // Resample to original block size
    PVresample (synthsamples.data(), synthlength, samples, numsamples);
}
