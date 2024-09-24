#pragma once

static const juce::String Semitones = "Semitones";

inline juce::AudioProcessorValueTreeState::ParameterLayout createParameters()
{
    juce::AudioProcessorValueTreeState::ParameterLayout parameters;
    
    parameters.add (std::make_unique<juce::AudioParameterInt> (juce::ParameterID (Semitones, 1), Semitones, -12.0f, 12.0f, 0.0f));
    
    return parameters;
}
