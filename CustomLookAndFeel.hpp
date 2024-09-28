#pragma once

#include <JuceHeader.h>

class CustomLookAndFeel : public juce::LookAndFeel_V4
{
public:
    CustomLookAndFeel();

    void drawTabButton(juce::TabBarButton& button, juce::Graphics& g, bool isMouseOver, bool isMouseDown) override;
    int getTabButtonBestWidth(juce::TabBarButton&, int tabDepth) override;
};
