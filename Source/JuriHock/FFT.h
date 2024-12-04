#pragma once

#if defined(ENABLE_POCKET_FFT)

    #include "FFT/PocketFFT.h"

typedef PocketFFT FFT;

#else

    #include "FFT/RealFFT.h"

typedef RealFFT FFT;

#endif
