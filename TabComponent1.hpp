#pragma once

#include <JuceHeader.h>
#include "YINAudioComponent.hpp"

class TabComponent1 : public juce::Component
{
public:
    TabComponent1();
    ~TabComponent1() override;

    void prepareToPlay(int samplesPerBlockExpected, double sampleRate);
    void releaseResources();
    
    void processAudioBuffer(const juce::AudioSourceChannelInfo& bufferToFill);

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    void detectChordFromPitches(const std::vector<float>& detectedPitches);

    juce::Label chordLabel; // Displays detected chord
    std::map<juce::String, std::vector<int>> chordTemplates; // Chord templates
    YINAudioComponent yinComponent;
    

    double sampleRate;
    const int stableFramesRequired = 15;
    static constexpr int MINIMUM_BUFFER_SIZE = 8192;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TabComponent1)
};
