#pragma once

#include "JuceHeader.h"
#include "YINAudioComponent.hpp"
#include "InfoOverlay.hpp"

class TabComponent2 : public juce::Component
{
public:
    TabComponent2();
    ~TabComponent2() override;

    void resized() override;
    void paint(juce::Graphics& g) override;


    void prepareToPlay(int samplesPerBlockExpected, double sampleRate);
    void processAudioBuffer(const juce::AudioSourceChannelInfo& bufferToFill);


    void updateNoteUI(const juce::String& message);
    void updateStatusUI(const juce::String& message);


    void resetChallenge();
    
    void releaseResources();

private:

    juce::Label noteLabel;
    juce::Label requiredNoteLabel;
    juce::ComboBox scaleComboBox;
    juce::TextButton resetButton;
    juce::Label statusLabel;


    YINAudioComponent yinProcessor;
    float lastFrequency;
    juce::String currentNote;
    juce::String currentRequiredNote;
    int currentNoteIndex;
    bool isCorrectNote;
    bool scaleCompleted;
    
    InfoOverlay infoOverlay;
    juce::TextButton infoButton;

    std::vector<juce::String> currentScaleNotes;
    std::vector<std::pair<juce::String, juce::String>> stringAndFret;

    int stabilityCounter;
    int requiredStabilityCount;


    juce::String getNoteNameFromFrequencyWithTolerance(float frequency);

    void checkNoteInScale(float frequency);
    void loadScale();
    void updateRequiredNote();
    void moveToNextNote();
    void toggleInfoOverlay();

    void placeComponent(juce::Component& comp, juce::Rectangle<int>& area, int height, int spacing);
    void showMessageWithDelay(const juce::String& message, int delay, std::function<void()> callback);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TabComponent2)
};
