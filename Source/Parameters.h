#pragma once
#include <JuceHeader.h>

enum AlgorithmChoice
{
    WangRubberband = 0,
    WangSoundTouch = 1,
    WangVocoder = 2,
    WubVocoder = 3,
    McPherson = 4,
    Dysomni = 5,
    JuriHock = 6,
    MineRubberband = 7,
};

static const juce::String Semitones = "Semitones";

static const juce::String Algorithm = "Algorithm";
static const juce::String WangRubberband_Algorithm = "WangRubberband";
static const juce::String WangSoundTouch_Algorithm = "WangSoundTouch";
static const juce::String WangVocoder_Algorithm = "WangVocoder";
static const juce::String WubVocoder_Algorithm = "WubVocoder";
static const juce::String McPherson_Algorithm = "McPherson";
static const juce::String Dysomni_Algorithm = "Dysomni";
static const juce::String JuriHock_Algorithm = "JuriHock";
static const juce::String MineRubberband_Algorithm = "MineRubberband";

static const juce::StringArray AlgorithmList (WangRubberband_Algorithm,
                                              WangSoundTouch_Algorithm,
                                              WangVocoder_Algorithm,
                                              WubVocoder_Algorithm,
                                              McPherson_Algorithm,
                                              Dysomni_Algorithm,
                                              JuriHock_Algorithm,
                                              MineRubberband_Algorithm);

inline juce::AudioProcessorValueTreeState::ParameterLayout createParameters()
{
    juce::AudioProcessorValueTreeState::ParameterLayout parameters;

    parameters.add (std::make_unique<juce::AudioParameterInt> (juce::ParameterID (Semitones, 1), Semitones, -12.0f, 12.0f, 0.0f));

    parameters.add (std::make_unique<juce::AudioParameterChoice> (juce::ParameterID (Algorithm, 1), Algorithm, AlgorithmList, 0));

    return parameters;
}
