#pragma once

#include "JuceHeader.h"
#include "InfoOverlay.hpp"

class TabComponent1 : public juce::Component
{
public:
    TabComponent1();
    ~TabComponent1() override = default;


    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    
    void loadChord();

    // UI components
    juce::Label chordLabel;
    juce::ComboBox chordComboBox;
    InfoOverlay infoOverlay;
    juce::TextButton infoButton;

    // Chord positions
    std::vector<std::pair<int, int>> currentChordPositions;
    std::vector<int> mutedStrings; 
    
    void toggleInfoOverlay();
    

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TabComponent1)
};
