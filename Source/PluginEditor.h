#pragma once
#include "PluginProcessor.h"
#include <JuceHeader.h>

class PitchShifterAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:

    PitchShifterAudioProcessorEditor (PitchShifterAudioProcessor&);
    ~PitchShifterAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:

    PitchShifterAudioProcessor& audioProcessor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PitchShifterAudioProcessorEditor)
};
