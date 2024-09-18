#pragma once

#include <JuceHeader.h>
#include <chrono>
#include <vector>

//==============================================================================
// TabComponent3 handles tempo detection and deviation feedback
class TabComponent3 : public juce::Component,
                      private juce::Timer
{
public:
    TabComponent3();
    ~TabComponent3() override = default;

    // JUCE component overrides
    void resized() override;
    void paint(juce::Graphics& g) override;

    // Audio processing methods
    void processAudioBuffer(const juce::AudioSourceChannelInfo& bufferToFill); // To detect magnitude peaks
    void prepareToPlay(int samplesPerBlockExpected, double sampleRate);

private:
    // UI Components
    juce::Label label;                  // Label for displaying the current tempo information
    juce::TextButton tapButton;         // Button for tapping to set tempo manually
    juce::Slider tempoSlider;           // Slider for setting manual tempo
    juce::Rectangle<int> visualCueArea; // Area for a visual representation of the tempo
    bool showVisualCue = false;         // Whether to show the visual cue

    // Variables for tempo detection
    std::vector<std::chrono::steady_clock::time_point> tapTimes;  // Times of taps or peaks
    double detectedTempo = 120.0;    // Detected or manually set tempo in BPM
    double currentTempo = 0.0;       // Tempo calculated from player’s input
    bool isDeviatingFromTempo = false; // True if the player is deviating from the set tempo
    bool isPlaying = false;          // True if the player is actively playing

    // Methods for tempo functionality
    void tapTempo();                 // Function called when tapButton is pressed
    void setManualTempo();           // Function to set manual tempo from tempoSlider
    void detectTempoFromPeaks(float magnitude); // Detect tempo from magnitude peaks
    bool isPlayerInTempo();          // Check if player is in tempo based on detected BPM

    // Timer callback to handle background updates and visual cue
    void timerCallback() override;

    // Audio input-related variables
    float magnitudeThreshold = 0.01f;  // Threshold to consider the player is playing
    float previousMagnitude = 0.0f;    // Store previous magnitude to detect peaks
    int sampleRate = 44100;            // Default sample rate

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TabComponent3)
};
