#include "DysomniPitchShifter.h"

float PitchShifter::processSample (float x)
{
    x = x * changeGain;

    float x1 = pitchDelay1.processSample (x, a1);
    float x2 = pitchDelay2.processSample (x, a2);
    float x3 = pitchDelay3.processSample (x, a3);

    float g1 = calcGain (a1);
    float g2 = calcGain (a2);
    float g3 = calcGain (a3);

    validateAngleBounds (a1);
    validateAngleBounds (a2);
    validateAngleBounds (a3);

    validateChange (true);

    return (g1 * x1 + g2 * x2 + g3 * x3) * (2.0f / 3.0f) * changeGain;
}

float PitchShifter::kindaProcessSample (float x)
{
    x = x * changeGain;

    float x1 = pitchDelay1.processSample (x, a1);
    float x2 = pitchDelay2.processSample (x, a2);
    float x3 = pitchDelay3.processSample (x, a3);

    float g1 = calcGain (a1);
    float g2 = calcGain (a2);
    float g3 = calcGain (a3);

    validateAngleBounds (a1);
    validateAngleBounds (a2);
    validateAngleBounds (a3);

    validateChange (false);

    return (g1 * x1 + g2 * x2 + g3 * x3) * (2.0f / 3.0f) * changeGain;
}

void PitchShifter::validateChange (bool on)
{
    if (! on)
    {
        if (changeGain == 0.0f && changeGainInc == 0.0f)
            return;

        changeGainInc = -GAIN_CHANGE_SPEED * 0.01f; // Factor más pequeño para transición suave
        changeGain += changeGainInc;

        // Usamos una condición menos agresiva para evitar saltos bruscos
        if (changeGain <= 0.01f)
        {
            changeGain = 0.0f;
            changeGainInc = 0.0f;
        }
    }
    else
    {
        if (changeGainInc < 0)
        {
            changeGain += changeGainInc;

            if (changeGain <= 0.01f)
            {
                changeGain = 0.0f;
                changeGainInc = GAIN_CHANGE_SPEED * 0.01f; // Iniciar cambio de pitch
                actualSetPitch (pitchBuffer);
            }
        }
        else if (changeGainInc > 0)
        {
            changeGain += changeGainInc;

            if (changeGain >= 0.99f)
            {
                changeGain = 1.0f;
                changeGainInc = 0.0f;
            }
        }
        else if (changeGainInc == 0.0f)
        {
            if (changeGain == 0.0f)
            {
                changeGainInc = GAIN_CHANGE_SPEED * 0.01f;
                changeGain += changeGainInc;
            }
        }
    }
}

void PitchShifter::setFs (float inFs)
{
    Fs = inFs;
    pitchDelay1.setFs (Fs);
    pitchDelay2.setFs (Fs);
    pitchDelay3.setFs (Fs);

    calcAngleChange();
}

void PitchShifter::setPitch (float inSemitone)
{
    if (semitone == inSemitone)
        return;

    pitchBuffer = inSemitone;
    changeGainInc = -GAIN_CHANGE_SPEED;
}

void PitchShifter::actualSetPitch (float inSemitone)
{
    semitone = inSemitone;

    // Activar crossfade en los PitchDelay para suavizar el cambio de pitch
    pitchDelay1.startCrossfade();
    pitchDelay2.startCrossfade();
    pitchDelay3.startCrossfade();

    calcDelta();
    calcAngleChange();

    pitchDelay1.setPitch (semitone);
    pitchDelay2.setPitch (semitone);
    pitchDelay3.setPitch (semitone);
}

void PitchShifter::calcDelta()
{
    tr = powf (2.0f, semitone / 12.0f);
    delta = 1.0f - tr;
}

void PitchShifter::calcAngleChange()
{
    freq = 1.0f / ((MAX_DELAY_SAMPLES - 1.0f) / (delta * Fs));
    angleChange = freq * M2_PI / Fs;
}

float PitchShifter::calcGain (float& angle)
{
    // Suavizar los picos de la onda sinusoidal, asegurando que el ángulo esté bien alineado
    float gain = 0.5f * sinf (angle) + 0.5f;

    // Aplicar un limitador suave para evitar saltos
    if (gain < 0.001f)
        gain = 0.0f;
    else if (gain > 0.999f)
        gain = 1.0f;

    return gain;
}

void PitchShifter::validateAngleBounds (float& angle)
{
    angle += angleChange;

    if (angle > M2_PI)
        angle -= M2_PI;
    else if (angle < 0.0f)
        angle += M2_PI;
}
