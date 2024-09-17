#pragma once

#include <JuceHeader.h>
#include "YINAudioComponent.hpp"

class TabComponent2 : public juce::Component,
                      private juce::Timer
{
public:
    TabComponent2();
    ~TabComponent2() override;

    void resized() override;
    void paint(juce::Graphics& g) override;

    // Audio processing
    void prepareToPlay(int samplesPerBlockExpected, double sampleRate);
    void processAudioBuffer(const juce::AudioSourceChannelInfo& bufferToFill);

    // UI update
    void updateNoteUI(const juce::String& detectedNote);

private:
    juce::Label noteLabel;                     // Label to display detected notes
    YINAudioComponent yinProcessor;            // YIN pitch detection processor

    float lastFrequency;                       // Last detected frequency
    juce::String currentNote;                  // Currently detected note
    std::vector<float> waveformBuffer;         // Buffer to store waveform for visualization

    // Timer callback to control repainting
    void timerCallback() override;

    // Convert frequency to note name with tolerance
    juce::String getNoteNameFromFrequencyWithTolerance(float frequency);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TabComponent2)
};
