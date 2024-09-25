#pragma once
#include <JuceHeader.h>

enum Algorithm
{
    Rubberband = 0,
    WubVocoder = 1,
    McPherson = 2,
    Dysomni = 3,
    JuriHock = 4,
};

static const juce::String Semitones = "Semitones";

static const juce::String Algorithm = "Algorithm";
static const juce::String Rubberband_Algorithm = "Rubberband";
static const juce::String WubVocoder_Algorithm = "WubVocoder";
static const juce::String McPherson_Algorithm = "McPherson";
static const juce::String Dysomni_Algorithm = "Dysomni";
static const juce::String JuriHock_Algorithm = "JuriHock";

inline juce::AudioProcessorValueTreeState::ParameterLayout createParameters()
{
    juce::AudioProcessorValueTreeState::ParameterLayout parameters;

    parameters.add (std::make_unique<juce::AudioParameterInt> (juce::ParameterID (Semitones, 1), Semitones, -12.0f, 12.0f, 0.0f));

    parameters.add (std::make_unique<juce::AudioParameterChoice> (juce::ParameterID (Algorithm, 1), Algorithm, juce::StringArray (Rubberband_Algorithm, WubVocoder_Algorithm, McPherson_Algorithm, Dysomni_Algorithm, JuriHock_Algorithm), 0));

    return parameters;
}
