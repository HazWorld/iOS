#include "CustomLookAndFeel.hpp"

CustomLookAndFeel::CustomLookAndFeel()
{
    // Customize the background color of the main window or other components
    setColour(juce::ResizableWindow::backgroundColourId, juce::Colours::white);
}

void CustomLookAndFeel::drawTabButton(juce::TabBarButton& button, juce::Graphics& g, bool isMouseOver, bool isMouseDown)
{
    auto area = button.getLocalBounds().toFloat();
    auto tabBackground = button.isFrontTab() ? juce::Colours::lightblue : button.getTabBackgroundColour().withMultipliedAlpha(isMouseOver ? 1.0f : 0.9f);

    g.setColour(tabBackground);
    g.fillRoundedRectangle(area, 15.0f);  // Rounded corners for the tab

    g.setColour(juce::Colours::black);
    g.setFont(18.0f);
    g.drawText(button.getButtonText(), area, juce::Justification::centred, true);
}

void CustomLookAndFeel::drawTabAreaBehindFrontButton(juce::TabbedButtonBar& bar, juce::Graphics& g, int w, int h)
{
    g.setColour(juce::Colours::lightgrey);  // Background color behind tabs
    g.fillRect(bar.getLocalBounds());
}
