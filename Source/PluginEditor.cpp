#include "PluginEditor.h"

PitchShifterAudioProcessorEditor::PitchShifterAudioProcessorEditor (PitchShifterAudioProcessor& p) : AudioProcessorEditor (&p), audioProcessor (p)
{
    setSize (400, 300);
}

PitchShifterAudioProcessorEditor::~PitchShifterAudioProcessorEditor() {}

void PitchShifterAudioProcessorEditor::paint (juce::Graphics&) {}

void PitchShifterAudioProcessorEditor::resized() {}
