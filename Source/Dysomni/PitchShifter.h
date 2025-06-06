#pragma once
#include "PitchDelay.h"
#include <JuceHeader.h>

class PitchShifter
{
public:

    PitchShifter() = default;
    ~PitchShifter() = default;

    float processSample (float x);
    float kindaProcessSample (float x);
    void setFs (float Fs);
    void setPitch (float semitone);
    void actualSetPitch (float semitone);
    void validateChange (bool on);

private:

    float changeGain { 0.0f };
    float pitchBuffer { 0.0f };
    float changeGainInc { 0.0f };
    float GAIN_CHANGE_SPEED { 0.05f };

    const float PI { juce::MathConstants<float>::pi };
    const float M2_PI { 2.0f * PI };

    float Fs { 48000.0f };

    PitchDelay pitchDelay1 { 1 };
    PitchDelay pitchDelay2 { 2 };
    PitchDelay pitchDelay3 { 3 };

    float a1 { 1.5f * PI };
    float a2 { (1.5f * PI) + (M2_PI / 3.f) };
    float a3 { (1.5f * PI) + (2.f * M2_PI / 3.f) };

    float angleChange;
    float freq;

    const float MAX_DELAY_SEC { 0.1f };
    float MAX_DELAY_SAMPLES { MAX_DELAY_SEC * Fs };

    float semitone { 0.0f };
    float tr { 1.0f }; // momentary trasposition
    float delta { 0.0f }; // rate of change for delay (samples)

    void calcDelta();
    void calcAngleChange();
    float calcGain (float& angle);
    void validateAngleBounds (float& angle);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PitchShifter)
};
