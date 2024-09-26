#pragma once

#include <JuceHeader.h>

class TabComponent1 : public juce::Component
{
public:
    TabComponent1();
    ~TabComponent1() override = default;

    // UI painting and layout
    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    // Helper function to load chord positions
    void loadChord();

    // UI components
    juce::Label chordLabel;        // Displays selected chord name
    juce::ComboBox chordComboBox;  // Dropdown to select chords

    // Chord positions
    std::vector<std::pair<int, int>> currentChordPositions; // Stores string and fret positions of the current chord
    std::vector<int> mutedStrings;  // Stores muted string numbers

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TabComponent1)
};
