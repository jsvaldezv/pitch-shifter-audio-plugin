#pragma once
#include <JuceHeader.h>
#include <cmath>

class PitchDelay
{
public:

    PitchDelay (int phaseChoice);
    ~PitchDelay() = default;

    float processSample (float x, float& angle);

    void setFs (float Fs);

    void setPitch (float semitone);

    void startCrossfade();

private:

    float Fs { 48000.0f };
    int phaseChoice;

    const float PI { juce::MathConstants<float>::pi };

    const float MAX_DELAY_SEC { 0.1f };
    float MAX_DELAY_SAMPLES { MAX_DELAY_SEC * Fs };
    float delay; // in samples

    const int MAX_BUFFER_SIZE { 96000 };
    float delayBuffer[96000] = { 0.0f };
    int index { 0 };

    float semitone { 0.0f };
    float tr { 1.0f }; // momentary trasposition
    float delta { 0.0f }; // rate of change for delay (samples)

    int oldIndex = 0; // Índice antiguo durante crossfade
    float oldDelay = 0.0f; // Delay anterior para el crossfade

    bool crossfadeActive = false; // Indicador si el crossfade está activo
    float crossfadePos = 0.0f; // Progreso del crossfade (0.0 a 1.0)
    float crossfadeStep = 0.01f; // Tamaño del paso para el crossfade (controla la duración)

    void calcDelay();
    void incDelay (float& angle);
    void addToBuffer (float& sample);

    float calcFractionalDelay();
    float calcFractionalDelay (float delayValue, int bufferIndex);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PitchDelay)
};
