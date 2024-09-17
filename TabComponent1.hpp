#pragma once

#include <JuceHeader.h>
#include <array>
#include <map>
#include "YINAudioComponent.hpp"

//==============================================================================
// Tab 1 header (Chord Detector)
class TabComponent1 : public juce::AudioAppComponent
{
public:
    TabComponent1();
    ~TabComponent1() override;

    // Audio-related methods
    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;
    
    // UI and graphical updates
    void resized() override;
    void paint(juce::Graphics& g) override;

private:
    juce::Label chordLabel;          // Label to display detected chords
    YINAudioComponent yinComponent;  // YIN pitch detection component
    double sampleRate = 0.0;         // Store the sample rate for audio processing

    // Chord templates for chord detection
    std::map<juce::String, std::vector<int>> chordTemplates;

    // Method to detect chords based on detected pitch from YIN
    void detectChordFromYIN(float detectedPitch);

    // Smoothed note magnitudes for chord stability
    static constexpr int stableFramesRequired = 10;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TabComponent1)
};
