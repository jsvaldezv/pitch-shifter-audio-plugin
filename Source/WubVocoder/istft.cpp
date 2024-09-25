#include "istft.h"

void ISTFT::init (int order)
{
    windowlen = int (pow (2.0f, order));
    hannwindow.assign (windowlen, 0.0f);
    sampleframe.assign (windowlen, 0.0f);
    fftoutput.assign (windowlen, 0.0f);
    float pi = 3.14159f;
    hannsum = 0.0f;
    for (int n = 0; n < windowlen; n++)
    {
        hannwindow[n] = sqrt (0.5f * (1.0f - cosf (2.0f * pi * float (n) / float (windowlen))));
        hannsum += hannwindow[n];
    }
    fft = std::make_unique<dsp::FFT> (order);
}

void ISTFT::step (Cfloat* fftinput, float* samples, int numsamples)
{
    // Enforce conjugate symmetry
    int m = windowlen - 1;
    int N = windowlen / 2;
    for (int n = 1; n < N; n++)
        fftinput[m--] = std::conj (fftinput[n]);
    fftinput[N] = std::real (fftinput[N]);
    // Compute inverse FFT
    fft->perform (fftinput, fftoutput.data(), true);
    // Add windowed output to sample frame
    for (int n = 0; n < windowlen; n++)
        sampleframe[n] += fftoutput[n].real() * hannwindow[n];
    // Get output samples
    for (int n = 0; n < numsamples; n++)
        samples[n] = sampleframe[n] * float (numsamples) / hannsum;
    // Shift sample frame
    for (int n = numsamples; n < windowlen; n++)
        sampleframe[n - numsamples] = sampleframe[n];
    for (int n = windowlen - numsamples; n < windowlen; n++)
        sampleframe[n] = 0.0f;
}
