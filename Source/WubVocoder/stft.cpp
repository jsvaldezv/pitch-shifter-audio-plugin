#include "stft.h"
#include <cmath>

void STFT::init (int order)
{
    windowlen = int (pow (2.0f, order));
    hannwindow.assign (windowlen, 0.0f);
    sampleframe.assign (windowlen, 0.0f);
    fftinput.assign (windowlen, 0.0f);
    float pi = 3.14159f;
    for (int n = 0; n < windowlen; n++)
        hannwindow[n] = sqrt (0.5f * (1.0f - cosf (2.0f * pi * float (n) / float (windowlen))));
    fft = std::make_unique<dsp::FFT> (order);
}

void STFT::step (float* samples, int numsamples, Cfloat* fftoutput)
{
    // Shift samples into frame
    for (int n = numsamples; n < windowlen; n++)
        sampleframe[n - numsamples] = sampleframe[n];
    for (int n = 0; n < numsamples; n++)
        sampleframe[windowlen - numsamples + n] = samples[n];
    // Apply window
    for (int n = 0; n < windowlen; n++)
        fftinput[n] = sampleframe[n] * hannwindow[n];
    // Compute FFT
    fft->perform (fftinput.data(), fftoutput, false);
}
