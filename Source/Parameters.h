#pragma once
#include <JuceHeader.h>

enum Algorithm
{
    Rubberband = 0,
    PhaseVocoder = 1
};

static const juce::String Semitones = "Semitones";

static const juce::String Algorithm = "Algorithm";
static const juce::String Rubberband_Algorithm = "Rubberband";
static const juce::String PhaseVocoder_Algorithm = "PhaseVocoder";

inline juce::AudioProcessorValueTreeState::ParameterLayout createParameters()
{
    juce::AudioProcessorValueTreeState::ParameterLayout parameters;

    parameters.add (std::make_unique<juce::AudioParameterInt> (juce::ParameterID (Semitones, 1), Semitones, -12.0f, 12.0f, 0.0f));

    parameters.add (std::make_unique<juce::AudioParameterChoice> (juce::ParameterID (Algorithm, 1), Algorithm, juce::StringArray (Rubberband_Algorithm, PhaseVocoder_Algorithm), 0));

    return parameters;
}
