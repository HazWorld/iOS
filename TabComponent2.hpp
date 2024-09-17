#pragma once

#include <JuceHeader.h>
#include <array>
#include <cmath>
#include "YINAudioComponent.hpp"

//==============================================================================
// Tab 2 header (Note Detector)
class TabComponent2 : public juce::AudioAppComponent
{
public:
    TabComponent2();
    ~TabComponent2() override;

    // Audio-related methods
    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override {}

    // UI methods
    void resized() override;
    void paint(juce::Graphics& g) override;

private:
    juce::Label noteLabel;                 // Label to display detected notes
    YINAudioComponent yinProcessor;        // YIN pitch detection processor
    juce::IIRFilter lowPassFilter;         // Low-pass filter for noise reduction

    float lastFrequency;                   // Last detected frequency
    float smoothedFrequency;               // Smoothed detected frequency
    int stableNoteHoldTime;                // Stability counter for note detection
    juce::String currentNote;              // Currently detected note

    // YIN-based note detection
    void detectNoteFromYIN(const float* audioBuffer, int numSamples);

    // Convert frequency to note name
    juce::String getNoteNameFromFrequency(float frequency);

    // Update the UI with the detected note
    void updateNoteUI(const juce::String& detectedNote);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TabComponent2)
};
