# Pitch Shifter in JUCE

## Introduction

The plugin is developed in C++ using the JUCE library. This plugin allows users to modify the pitch of an audio signal within a range of +/- 12 semitones, offering flexibility for applying effects like harmonization, pitch shifting, or tuning correction. Additionally, users can choose between different pitch-shifting algorithms to suit their creative or technical needs.

The plugin is designed to be intuitive, offering a simple interface to select the number of semitones and the algorithm type. This makes it ideal for both beginners and audio professionals looking for a powerful solution to manipulate the pitch of their recordings.

## Algorithms

This plugin includes several pitch-shifting algorithms. Each algorithm has its own strengths and is based on open-source contributions from various repositories, as well as a custom implementation using the **Rubberband** and **SoundTouch** library. Below is the list of algorithms and their respective credits:

1. **Rubberband Pitch Shifter (Wang)**
   - [WangRubberBandPitchShifter](https://github.com/wangchengzhong/Voice-Changer/blob/master/Source/PitchShifterRubberband.h)  
   - This algorithm is based on Rubberband, which is ideal for high-quality pitch shifting without affecting the timing.

2. **SoundTouch Pitch Shifter (Wang)**
   - [WangSoundTouchPitchShifter](https://github.com/wangchengzhong/Voice-Changer/blob/master/Source/PitchShifterSoundTouch.h)  
   - A granular-based approach using SoundTouch for efficient time-stretching and pitch shifting.

3. **Vocoder Pitch Shifter (Wang)**
   - [WangVocoderPitchShifter](https://github.com/wangchengzhong/Voice-Changer/blob/master/Source/PitchShifter.h)  
   - A vocoder-based pitch-shifting algorithm, ideal for complex pitch modifications with harmonic preservation.

4. **FFT-Based Pitch Shifter (ProfessorWub)**
   - [WubPitchShifter](https://github.com/professorwub/pitchshifter)  
   - Uses Fast Fourier Transform to achieve efficient and precise pitch shifting in the frequency domain.

5. **Phase Vocoder Pitch Shifter (McPherson)**
   - [McPhersonPitchShifter](https://github.com/juandagilc/Audio-Effects)  
   - Phase vocoder approach for smooth pitch shifting, ideal for maintaining high audio fidelity.

6. **Harmonizer Pitch Shifter (Dysomni)**
   - [DysomniPitchShifter](https://github.com/dysomni/Harmonizer)  
   - Harmonizer-based pitch shifter for creating harmony effects while modifying pitch.

7. **STFT Pitch Shifter (Juri Hock)**
   - [StftPitchShifter](https://github.com/jurihock/stftPitchShiftPlugin)  
   - A Short-Time Fourier Transform (STFT)-based algorithm, allowing fine control over pitch shifts while preserving time and phase relationships.

8. **Custom Rubberband Pitch Shifter (Own Implementation)**
   - `MineRubberbandPitchShifter`  
   - This is a custom implementation using the **Rubberband** library for high-quality pitch shifting. It offers flexibility in pitch modification while maintaining the original time length.

---

## Requirements

Before building the Pitch Shifter Plugin, ensure you have the following installed:

- **CMake**: A cross-platform build system to generate build files. Install CMake from [here](https://cmake.org/install/).
- **Xcode (for MacOS)**: App Store.
- **Visual Studio 2022 (for Windows)**: [https://visualstudio.microsoft.com/es/](https://visualstudio.microsoft.com).

## Cloning the Repository and Submodules

Clone the repository along with the required submodules:

```bash
git clone https://github.com/jsvaldezv/pitch-shifter.git
cd pitch-shifter
git submodule update --init --recursive
```

The following submodules are required for the plugin to work:

- **JUCE Framework**: Provides the core functionality for the plugin. Clone from JUCE GitHub Repository.
- **STFT Pitch Shifter**: An algorithm that relies on Short-Time Fourier Transform (STFT). Clone from Juri Hock's STFT Pitch Shift Plugin.

Ensure these submodules are initialized by running:

```bash
git submodule update --init --recursive
```

---

## Steps for Integrating the Algorithms

The following steps describe how to integrate the different pitch-shifting algorithms into your plugin using JUCE. This includes importing the necessary external files, initializing and preparing the processing objects, setting the number of semitones, and processing the audio signal.

### 1. Include the Desired Pitch Shifter Algorithm in Processor.h

Depending on the algorithm you want to use, you need to add the corresponding #include statement in the Processor.h file. For example:

```bash
// For Dysomni Pitch Shifter
#include "Dysomni/DysomniPitchShifter.h"

// For Wang Rubberband Pitch Shifter
#include "WangRubberband/WangRubberBandPitchShifter.h"
```

### 2. Create the Pitch Shifter Object in Processor.h

Once the include is in place, create the pitch shifter object as a member of your processor class. For example:

```bash
class MyAudioProcessor : public juce::AudioProcessor
{
public:
    // Pitch shifter object declaration
    WangRubberBandPitchShifter wangRubberBandPitchShifter;
    
    // If using a different algorithm:
    // DysomniPitchShifter dysomniPitchShifter;
    // WubPitchShifter wubPitchShifter;
};
```

### 3. Prepare the Pitch Shifter in prepareToPlay

In the prepareToPlay method, you'll configure the pitch shifter object using the juce::dsp::ProcessSpec. This ensures that the pitch shifter is initialized with the correct sample rate, block size, and number of channels.

```bash
void prepareToPlay (double sampleRate, int samplesPerBlock) override
{
    juce::dsp::ProcessSpec spec;
    spec.maximumBlockSize = (juce::uint32) samplesPerBlock;
    spec.numChannels = (juce::uint32) getTotalNumOutputChannels();
    spec.sampleRate = sampleRate;

    // Prepare the pitch shifter
    wangRubberBandPitchShifter.prepare (spec, true, true);

    // For other pitch shifters, adjust accordingly:
    // dysomniPitchShifter.prepare (spec);
    // wubPitchShifter.prepare (spec);
}
```

### 4. Set the Semitones for Pitch Shifting

You can set the number of semitones that the pitch shifter should shift by using the setSemitones method. This should be done whenever the pitch shift needs to be updated, such as from a UI control.

```bash
void setPitchShifting (int currentSemitones)
{
    wangRubberBandPitchShifter.setSemitones (currentSemitones);

    // For other pitch shifters:
    // dysomniPitchShifter.setSemitones(currentSemitones);
    // wubPitchShifter.setSemitones(currentSemitones);
}
```

### 5. Process Audio in processBlock

In the processBlock method, pass the audio buffer to the pitch shifter to apply the effect. This is where the pitch-shifting is applied in real-time as the audio is processed.

```bash
void processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&) override
{
    // Process the audio buffer with the pitch shifter
    wangRubberBandPitchShifter.process (buffer);

    // For other pitch shifters:
    // dysomniPitchShifter.process(buffer);
    // wubPitchShifter.process(buffer);
}
```
