#pragma once

#include <JuceHeader.h>

// Custom Look and Feel Class Declaration
class CustomLookAndFeel : public juce::LookAndFeel_V4
{
public:
    CustomLookAndFeel();

    // Custom tab button drawing
    void drawTabButton(juce::TabBarButton& button, juce::Graphics& g);

    // Background behind tab buttons
    void drawTabAreaBehindFrontButton(juce::TabbedButtonBar& bar, juce::Graphics& g, int w, int h) override;

    // Method to control the tab width
    int getTabButtonBestWidth(juce::TabBarButton& button, int tabDepth) override;

};
