#pragma once

#include <JuceHeader.h>

class TabComponent3 : public juce::Component,
                      public juce::Timer
{
public:
    TabComponent3();
    ~TabComponent3() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;

    // Audio processing
    void processAudioBuffer(const juce::AudioSourceChannelInfo& bufferToFill);
    void prepareToPlay(int samplesPerBlockExpected, double sampleRate);
    
    void releaseResources();

private:
    // UI components
    juce::Label label;
    juce::TextButton tapButton;
    juce::Slider tempoSlider;

    // Tempo detection
    std::vector<std::chrono::steady_clock::time_point> tapTimes;
    double detectedTempo { 120.0 };  // Default tempo in BPM
    double currentTempo { 0.0 };
    float dynamicThreshold { 0.05f };
    float smoothedMagnitude { 0.0f };
    float previousMagnitude { 0.0f };
    bool isPlaying { false };
    bool isDeviatingFromTempo { false };
    bool showVisualCue { false };
    int sampleRate;
    std::chrono::steady_clock::time_point lastDebounceTime;
    std::chrono::steady_clock::time_point lastPeakTime;

    // FlexBox Layout
    juce::Rectangle<int> visualCueArea;

    void tapTempo();
    void setManualTempo();
    void detectTempoFromPeaks(float magnitude);
    void adjustThreshold(float magnitude);
    bool isPlayerInTempo();
    void timerCallback() override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TabComponent3)
};
