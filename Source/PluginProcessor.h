#pragma once
#include "Parameters.h"
#include "Rubberband/RubberBandPitchShifter.h"
#include "WubVocoder/PitchShifter.h"
#include "McPherson/McPhersonPitchShifter.h"
#include <JuceHeader.h>

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
    
    float currentSemitones { 0.0f };

    std::unique_ptr<RubberbandPitchShifter> rubberbandPitchShifter;
    
    std::vector<std::unique_ptr<WubVocoderPitchShifter>> wubVocoderPitchShifter; // https://github.com/professorwub/pitchshifter
    float shifterHopSize { 0.0f };
    
    McPhersonPitchShifter mcPhersonPitchShifter;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PitchShifterAudioProcessor)
};
