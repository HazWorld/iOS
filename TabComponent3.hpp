#pragma once

#include "JuceHeader.h"
#include "InfoOverlay.hpp"
#include <chrono>
#include <vector>

class TabComponent3 : public juce::Component
{
public:
    TabComponent3();
    ~TabComponent3() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;

    void processAudioBuffer(const juce::AudioSourceChannelInfo& bufferToFill);
    void prepareToPlay(int samplesPerBlockExpected, double sampleRate);
    void releaseResources();

private:
    //UI
    juce::Label detectedTempoLabel;
    juce::Label setTempoLabel;
    juce::Slider tempoSlider;

    //tempo
    std::vector<std::chrono::steady_clock::time_point> tapTimes;
    double detectedTempo { 120.0 };
    double currentTempo { 0.0 };
    float dynamicThreshold { 0.05f };
    float smoothedMagnitude { 0.0f };
    float previousMagnitude { 0.0f };
    int sampleRate { 44100 };
    
    //info button
    InfoOverlay infoOverlay;
    juce::TextButton infoButton;


    std::chrono::steady_clock::time_point lastPeakTime;

    void setManualTempo();
    void detectTempoFromPeaks(float magnitude);
    void adjustThreshold(float magnitude);
    
    void toggleInfoOverlay();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TabComponent3)
};
