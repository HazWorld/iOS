#pragma once

#include <JuceHeader.h>
#include <array>
#include <vector>
#include "FFTAudioComponent.hpp"

//==============================================================================
// TabComponent2 header (Note Detector)
class TabComponent2 : public FFTAudioComponent, private juce::Timer
{
public:
    TabComponent2();
    ~TabComponent2() override;

    void prepareToPlay(int samplesPerBlockExpected, double sampleRate);
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill);
    void releaseResources();
    void resized() override;
    void paint(juce::Graphics& g) override;

private:
    juce::Label noteLabel;

    // Timer callback to update the graph
    void timerCallback() override;

    // Note detection and smoothing
    void detectNoteFromFFT();
    juce::String getNoteNameFromFrequency(float frequency);

    // Dynamic noise floor calculation
    float calculateNoiseFloor();

    // Smoothing and hysteresis parameters for note detection stability
    float lastFrequency = 0.0f;
    float smoothedFrequency = 0.0f;
    int stableNoteHoldTime = 0;
    juce::String currentNote = "Unknown";

    // Harmonics detection for fundamental frequency validation
    const int maxHarmonics = 5;
    
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TabComponent2)
};
