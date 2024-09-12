
#pragma once

#include <JuceHeader.h>
#include <array>
#include <vector>
#include "FFTAudioComponent.hpp"

//==============================================================================
// Tab 1 header (Chord Detector)
class TabComponent1 : public FFTAudioComponent, private juce::Timer
{
public:
    TabComponent1();
    ~TabComponent1() override;

    void prepareToPlay(int samplesPerBlockExpected, double sampleRate);
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill);
    void releaseResources();
    void resized() override;
    void paint(juce::Graphics& g) override;

private:
    juce::Label chordLabel;

    // Timer callback to update the graph
    void timerCallback() override;
    
    std::map<juce::String, std::vector<int>> chordTemplates;

    void detectChordFromFFT();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TabComponent1)
};
