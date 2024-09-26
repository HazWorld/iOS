#pragma once

#include <JuceHeader.h>
#include "YINAudioComponent.hpp"
#include "InfoOverlay.hpp"

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
    void updateStatusUI(const juce::String& message);

    // Reset the challenge
    void resetChallenge();
    
    void releaseResources();

private:
    // UI Components
    juce::Label noteLabel;
    juce::Label requiredNoteLabel;
    juce::ComboBox scaleComboBox;
    juce::TextButton resetButton;
    juce::Label statusLabel;

    // Audio and Note Detection
    YINAudioComponent yinProcessor;
    float lastFrequency;
    juce::String currentNote;
    juce::String currentRequiredNote;
    int currentNoteIndex;
    bool isCorrectNote;
    
    InfoOverlay infoOverlay;
    juce::TextButton infoButton;

    // Scale Data
    std::vector<juce::String> currentScaleNotes;
    std::vector<std::pair<juce::String, juce::String>> stringAndFret;

    int stabilityCounter;
    int requiredStabilityCount;


    void timerCallback() override;

    juce::String getNoteNameFromFrequencyWithTolerance(float frequency);

    void checkNoteInScale(float frequency);

    void loadScale();

    void updateRequiredNote();

    void placeComponent(juce::Component& comp, juce::Rectangle<int>& area, int height, int spacing);
    
    void showMessageWithDelay(const juce::String& message, int delay, std::function<void()> callback);
    
    void moveToNextNote();
    
    void toggleInfoOverlay();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TabComponent2)
};
