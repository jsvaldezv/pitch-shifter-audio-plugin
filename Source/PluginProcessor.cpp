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
    int numChannels = getTotalNumOutputChannels();
    
    juce::dsp::ProcessSpec spec;
    spec.maximumBlockSize = (juce::uint32) samplesPerBlock;
    spec.numChannels = (juce::uint32) getTotalNumOutputChannels();
    spec.sampleRate = sampleRate;

    // Pitch Rubberband
    rubberbandPitchShifter = std::make_unique<RubberbandPitchShifter> (getTotalNumOutputChannels(), sampleRate, samplesPerBlock, true, true);
    rubberbandPitchShifter->setMixPercentage (100.0f);

    // Pitch Wub Vocoder
    int minWindowLength = 16 * samplesPerBlock;
    int order = 0;
    int windowLength = 1;
    while (windowLength < minWindowLength)
    {
        order++;
        windowLength *= 2;
    }

    wubVocoderPitchShifter.resize ((size_t) numChannels);

    for (size_t ch = 0; ch < (size_t) numChannels; ch++)
    {
        wubVocoderPitchShifter[ch] = std::unique_ptr<WubVocoderPitchShifter> (new WubVocoderPitchShifter);
        wubVocoderPitchShifter[ch]->init (order);
    }

    // McPherson
    mcPhersonPitchShifter.preparePitch ((int) spec.sampleRate, (int) spec.numChannels);

    // Dysomni
    for (int ch = 0; ch < numChannels; ch++)
        dysomniPitchShifter[ch].setFs ((float) sampleRate);
    
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

    int numSamples = buffer.getNumSamples();
    updateParameters();

    switch ((int) apvts.getRawParameterValue (Algorithm)->load())
    {
        case Algorithm::Rubberband:
            rubberbandPitchShifter->processBuffer (buffer);
            break;

        case Algorithm::WubVocoder:
            for (size_t ch = 0; ch < (size_t) buffer.getNumChannels(); ch++)
                wubVocoderPitchShifter[ch]->step (buffer.getWritePointer ((int) ch), numSamples, wubShifterHopSize);
            break;

        case Algorithm::McPherson:
            mcPhersonPitchShifter.processPitchShifting (buffer, currentSemitones);
            break;

        case Algorithm::Dysomni:
            for (int channel = 0; channel < buffer.getNumChannels(); channel++)
            {
                for (int i = 0; i < buffer.getNumSamples(); i++)
                {
                    float sample = buffer.getSample (channel, i);
                    float out = dysomniPitchShifter[channel].processSample (sample);
                    buffer.setSample (channel, i, out);
                }
            }
            break;
    }
    
    //townleyPitchShifter.process (buffer);
}

void PitchShifterAudioProcessor::updateParameters()
{
    currentSemitones = apvts.getRawParameterValue (Semitones)->load();

    rubberbandPitchShifter->setSemitoneShift (currentSemitones);

    wubShifterHopSize = std::pow (2.0f, currentSemitones / 12.0f);

    for (int channel = 0; channel < getTotalNumOutputChannels(); channel++)
        dysomniPitchShifter[channel].setPitch (currentSemitones);
    
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
