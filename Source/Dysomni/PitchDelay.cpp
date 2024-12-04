#include "PitchDelay.h"

PitchDelay::PitchDelay (int inPhaseChoice)
{
    phaseChoice = inPhaseChoice;
    calcDelay();
}

float PitchDelay::processSample (float x, float& angle)
{
    addToBuffer (x);
    incDelay (angle);

    // Si está en proceso de crossfade, aplicamos la transición
    if (crossfadeActive)
    {
        float sampleA = calcFractionalDelay (oldDelay, oldIndex);
        float sampleB = calcFractionalDelay (delay, index);

        // Realizamos el crossfade entre las dos señales
        float crossfadedSample = (1.0f - crossfadePos) * sampleA + crossfadePos * sampleB;

        // Incrementamos la posición del crossfade
        crossfadePos += crossfadeStep;

        // Cuando el crossfade termina, lo desactivamos
        if (crossfadePos >= 1.0f)
        {
            crossfadeActive = false;
            crossfadePos = 0.0f;
        }

        return crossfadedSample;
    }
    else
    {
        // Procesamiento normal sin crossfade
        return calcFractionalDelay (delay, index);
    }
}

void PitchDelay::setFs (float inFs)
{
    Fs = inFs;
    MAX_DELAY_SAMPLES = MAX_DELAY_SEC * Fs;
    calcDelay();
}

void PitchDelay::setPitch (float inSemitone)
{
    semitone = inSemitone;

    tr = std::powf (2.0f, semitone / 12.0f);

    delta = 1.0f - tr;

    // Iniciar el crossfade cuando el pitch cambia
    startCrossfade();
}

void PitchDelay::addToBuffer (float& sample)
{
    delayBuffer[index] = sample;

    // Increment buffer
    if (index < MAX_BUFFER_SIZE - 1)
        index++;
    else
        index = 0;
}

void PitchDelay::calcDelay()
{
    switch (phaseChoice)
    {
        case 1:
            delay = 2.0f;
            break;
        case 2:
            delay = (float) MAX_DELAY_SAMPLES / 3.0f;
            break;
        case 3:
            delay = 2.0f * (float) MAX_DELAY_SAMPLES / 3.0f;
            break;
    }
}

void PitchDelay::incDelay (float& angle)
{
    delay += delta;

    if (delta <= 0.0f && delay < 2.0f)
    {
        delay = (float) MAX_DELAY_SAMPLES;
        angle = 1.5f * PI;
    }

    if (delta > 0.0f && delay > MAX_DELAY_SAMPLES)
    {
        delay = 2.0f;
        angle = 1.5f * PI;
    }
}

float PitchDelay::calcFractionalDelay (float delayValue, int bufferIndex)
{
    int d1 = static_cast<int> (std::floor (delayValue));
    int d2 = (d1 + 1) % MAX_BUFFER_SIZE;

    float frac = delayValue - static_cast<float> (d1);
    float g1 = 1.0f - frac;
    float g2 = frac;

    int indexD1 = (bufferIndex - d1 + MAX_BUFFER_SIZE) % MAX_BUFFER_SIZE;
    int indexD2 = (bufferIndex - d2 + MAX_BUFFER_SIZE) % MAX_BUFFER_SIZE;

    return g1 * delayBuffer[indexD1] + g2 * delayBuffer[indexD2];
}

float PitchDelay::calcFractionalDelay()
{
    int d1 = (int) (std::floor (delay));
    int d2 = d1 + 1;

    float g2 = delay - (float) d1;
    float g1 = 1.0f - g2;

    int indexD1 = index - d1;
    if (indexD1 < 0)
        indexD1 += MAX_BUFFER_SIZE;

    int indexD2 = index - d2;
    if (indexD2 < 0)
        indexD2 += MAX_BUFFER_SIZE;

    return g1 * delayBuffer[indexD1] + g2 * delayBuffer[indexD2];
}

void PitchDelay::startCrossfade()
{
    // Guardamos el estado actual del delay antes de cambiar
    oldDelay = delay;
    oldIndex = index;

    // Activamos el crossfade
    crossfadeActive = true;
    crossfadePos = 0.0f;

    // Definimos la velocidad del crossfade (aquí, 100 muestras)
    crossfadeStep = 1.0f / 100.0f;
}
