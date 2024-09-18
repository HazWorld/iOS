#pragma once

#include <JuceHeader.h>
#include "YINAudioComponent.hpp"

class TabComponent2 : public juce::Component,
                      private juce::Timer
{
public:
    TabComponent2();
    ~TabComponent2() override;

    // JUCE component overrides
    void resized() override;
    void paint(juce::Graphics& g) override;

    // Audio processing
    void prepareToPlay(int samplesPerBlockExpected, double sampleRate);
    void processAudioBuffer(const juce::AudioSourceChannelInfo& bufferToFill);

    // UI update
    void updateNoteUI(const juce::String& message);

    // Reset the learning challenge
    void resetChallenge();

private:
    // UI Components
    juce::Label noteLabel;                    // Displays the detected note feedback
    juce::Label requiredNoteLabel;            // Displays the next required note and the scale
    juce::ComboBox scaleComboBox;             // Allows the user to select a guitar scale
    juce::TextButton resetButton;             // Button to reset the scale challenge

    // Audio and Note Detection
    YINAudioComponent yinProcessor;           // YIN-based pitch detection processor
    float lastFrequency;                      // Last detected frequency
    juce::String currentNote;                 // Currently detected note
    juce::String currentRequiredNote;         // The note the user needs to play
    int currentNoteIndex;                     // Tracks the current note index in the scale being played
    bool isCorrectNote;                       // Tracks if the correct note was played

    // Scale Data Structures
    std::vector<juce::String> currentScaleNotes;   // The notes in the selected scale
    std::vector<std::pair<juce::String, juce::String>> stringAndFret;  // String and fret info for each note

    // Stability mechanism for note detection
    int stabilityCounter;                     // Counter to track how long the detected note is stable
    int requiredStabilityCount;               // How many frames the note must be stable for confirmation

    // Timer callback to control repainting
    void timerCallback() override;

    // Convert the detected frequency to a note name, with tolerance for small variations
    juce::String getNoteNameFromFrequencyWithTolerance(float frequency);

    // Check if the detected note matches the current required note in the scale
    void checkNoteInScale(float frequency);

    // Handle scale selection from the ComboBox
    void loadScale();

    // Update the next required note in the UI
    void updateRequiredNote();

    // Reset the note labels to their default state (e.g., color and text)
    void resetLabelsToDefault();

    // Helper function to position UI components
    void placeComponent(juce::Component& comp, juce::Rectangle<int>& area, int height, int spacing);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TabComponent2)
};
