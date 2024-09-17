#pragma once

#include <JuceHeader.h>
#include "TabComponent1.hpp"
#include "TabComponent2.hpp"
#include "TabComponent3.hpp"

// Custom Look and Feel Class
class CustomLookAndFeel : public juce::LookAndFeel_V4
{
public:
    CustomLookAndFeel()
    {
        // Customize the background color of the main window or other components
        setColour(juce::ResizableWindow::backgroundColourId, juce::Colours::white);
    }

    // Customize how the tab buttons are drawn
    void drawTabButton(juce::TabBarButton& button, juce::Graphics& g, bool isMouseOver, bool isMouseDown) override
    {
        auto area = button.getLocalBounds().toFloat();
        auto tabBackground = button.getTabBackgroundColour();  // Get the tab-specific background color

        // Draw rounded rectangle as the tab background
        g.setColour(tabBackground.withMultipliedAlpha(isMouseOver ? 1.0f : 0.8f));
        g.fillRoundedRectangle(area, 10.0f);  // Rounded corners

        // Draw the tab label
        g.setColour(juce::Colours::black);
        g.setFont(16.0f);
        g.drawText(button.getButtonText(), area, juce::Justification::centred, true);
    }

    // Customize tab button font and size
    void drawTabAreaBehindFrontButton(juce::TabbedButtonBar& bar, juce::Graphics& g, int w, int h) override
    {
        g.setColour(juce::Colours::lightgrey);  // Background color behind tabs
        g.fillRect(bar.getLocalBounds());
    }
};

// MainComponent declaration
class MainComponent : public juce::AudioAppComponent
{
public:
    MainComponent();
    ~MainComponent() override;

    void resized() override;
    void paint(juce::Graphics& g) override;

    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;

private:
    juce::TabbedComponent tabs;
    TabComponent1 tab1;
    TabComponent2 tab2;
    TabComponent3 tab3;

    CustomLookAndFeel customLookAndFeel;  // Custom look-and-feel instance

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
