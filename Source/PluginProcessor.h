#pragma once
#include "Dysomni/DysomniPitchShifter.h"
#include "JuriHock/StftPitchShifter.h"
#include "McPherson/McPhersonPitchShifter.h"
#include "Parameters.h"
#include "Townley/TownleyPitchShifter.h"
#include "WangRubberband/WangRubberBandPitchShifter.h"
#include "WubVocoder/WubPitchShifter.h"

class PitchShifterAudioProcessor : public juce::AudioProcessor
{
public:

    PitchShifterAudioProcessor();
    ~PitchShifterAudioProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

#ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
#endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;
    const juce::String getName() const override;
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

private:

    juce::AudioProcessorValueTreeState apvts { *this, nullptr, "Parameters", createParameters() };
    void updateParameters();

    int currentSemitones { 0 };

    // Algorithms
    WangRubberBandPitchShifter wangPitchShifter; // https://github.com/wangchengzhong/Voice-Changer/blob/master/Source/PitchShifterRubberband.h

    WubPitchShifter wubPitchShifter; // https://github.com/professorwub/pitchshifter

    McPhersonPitchShifter mcPhersonPitchShifter; // https://github.com/juandagilc/Audio-Effects

    DysomniPitchShifter dysomniPitchShifter; // https://github.com/dysomni/Harmonizer

    StftPitchShifter juriHockPitchShifter; // https://github.com/jurihock/stftPitchShiftPlugin

    // Pending...
    //TownleyPitchShifter townleyPitchShifter; // https://github.com/MartinTownley/JUCE_VDL_Pitch-Shifter

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PitchShifterAudioProcessor)
};
