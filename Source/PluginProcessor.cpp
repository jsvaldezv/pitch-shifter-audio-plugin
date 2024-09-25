#include "PluginProcessor.h"
#include "PluginEditor.h"

PitchShifterAudioProcessor::PitchShifterAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor (BusesProperties()
                          .withInput ("Input", juce::AudioChannelSet::stereo(), true)
                          .withOutput ("Output", juce::AudioChannelSet::stereo(), true))
#endif
{
}

PitchShifterAudioProcessor::~PitchShifterAudioProcessor() {}

const juce::String PitchShifterAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool PitchShifterAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool PitchShifterAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool PitchShifterAudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double PitchShifterAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int PitchShifterAudioProcessor::getNumPrograms()
{
    return 1;
}

int PitchShifterAudioProcessor::getCurrentProgram()
{
    return 0;
}

void PitchShifterAudioProcessor::setCurrentProgram (int /*index*/) {}

const juce::String PitchShifterAudioProcessor::getProgramName (int /*index*/)
{
    return {};
}

void PitchShifterAudioProcessor::changeProgramName (int /*index*/, const juce::String& /*newName*/) {}

void PitchShifterAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::dsp::ProcessSpec spec;
    spec.maximumBlockSize = (juce::uint32) samplesPerBlock;
    spec.numChannels = (juce::uint32) getTotalNumOutputChannels();
    spec.sampleRate = sampleRate;

    wangPitchShifter.prepare (spec, true, true);
    wangSoundTouchPitchShifter.prepare (spec, true, true);
    wangVocoder.prepare (spec);
    wubPitchShifter.prepare (spec);
    mcPhersonPitchShifter.prepare (spec);
    dysomniPitchShifter.prepare (spec);

    juriHockPitchShifter.prepare (spec);
    setLatencySamples (juriHockPitchShifter.getLatency());

    // Townley
    //townleyPitchShifter.prepare (spec);
}

void PitchShifterAudioProcessor::releaseResources() {}

#ifndef JucePlugin_PreferredChannelConfigurations
bool PitchShifterAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
    #else
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;
        #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
        #endif
    return true;
    #endif
}
#endif

void PitchShifterAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& /*midiMessages*/)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    updateParameters();

    switch ((int) apvts.getRawParameterValue (Algorithm)->load())
    {
        case Algorithm::WangRubberband:
            wangPitchShifter.process (buffer);
            break;

        case Algorithm::WangSoundTouch:
            wangSoundTouchPitchShifter.process (buffer);
            break;
            
        case Algorithm::WangVocoder:
            wangVocoder.process (buffer);

        case Algorithm::WubVocoder:
            wubPitchShifter.process (buffer);
            break;

        case Algorithm::McPherson:
            mcPhersonPitchShifter.process (buffer);
            break;

        case Algorithm::Dysomni:
            dysomniPitchShifter.process (buffer);
            break;

        case Algorithm::JuriHock:
            juriHockPitchShifter.process (buffer);

            if (juriHockPitchShifter.latencyChanged)
                setLatencySamples (juriHockPitchShifter.getLatency());

            break;
    }

    //townleyPitchShifter.process (buffer);
}

void PitchShifterAudioProcessor::updateParameters()
{
    currentSemitones = (int) apvts.getRawParameterValue (Semitones)->load();

    wangPitchShifter.setSemitones (currentSemitones);

    wangSoundTouchPitchShifter.setSemitones (currentSemitones);

    wubPitchShifter.setSemitones (currentSemitones);

    mcPhersonPitchShifter.setSemitones (currentSemitones);

    dysomniPitchShifter.setSemitones (currentSemitones);

    juriHockPitchShifter.setSemitones (currentSemitones);

    //townleyPitchShifter.setSemitones (currentSemitones);
}

bool PitchShifterAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* PitchShifterAudioProcessor::createEditor()
{
    //return new PitchShifterAudioProcessorEditor (*this);
    return new juce::GenericAudioProcessorEditor (*this);
}

void PitchShifterAudioProcessor::getStateInformation (juce::MemoryBlock& /*destData*/) {}

void PitchShifterAudioProcessor::setStateInformation (const void* /*data*/, int /*sizeInBytes*/) {}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PitchShifterAudioProcessor();
}
