#include "CustomLookAndFeel.hpp"

CustomLookAndFeel::CustomLookAndFeel()
{
    setColour(juce::ResizableWindow::backgroundColourId, juce::Colours::white);
}

void CustomLookAndFeel::drawTabButton(juce::TabBarButton& button, juce::Graphics& g)
{
    auto area = button.getLocalBounds().toFloat();
    float cornerSize = 10.0f;

    // Define background color based on the selected state of the tab
    juce::Colour tabBackground;
    
    if (button.isFrontTab())  // If the tab is selected (front tab)
    {
        // Use the assigned tab background color when selected
        tabBackground = button.getTabBackgroundColour();
    }
    else  // Non-selected tabs
    {
        // Grayed-out background for non-selected tabs with less transparency for better contrast
        tabBackground = juce::Colours::lightgrey.withAlpha(0.9f);  // Adjust the alpha to make non-selected tabs more visible
    }

    // Fill the tab background
    g.setColour(tabBackground);
    g.fillRoundedRectangle(area, cornerSize);

    // Add an outline to all tabs
//    g.setColour(juce::Colours::darkgrey);
//    g.drawRoundedRectangle(area.reduced(1), cornerSize, 1.0f);

    // Set the text color: Make sure it's dark for non-selected tabs
    juce::Colour textColor = button.isFrontTab() ? juce::Colours::white : juce::Colours::black;  // Use black for non-selected tabs
    g.setColour(textColor);

    // Apply bold font to the front tab and regular to others
    g.setFont(button.isFrontTab() ? juce::FontOptions(18.0f, juce::Font::bold) : juce::FontOptions(16.0f));

    // Draw the tab label, centered in the area
    g.drawText(button.getButtonText(), area.reduced(10), juce::Justification::centred, true);
}

// This method controls the tab width.
int CustomLookAndFeel::getTabButtonBestWidth(juce::TabBarButton&, int tabDepth)
{
    return 150;  // Wider tabs
}

void CustomLookAndFeel::drawTabAreaBehindFrontButton(juce::TabbedButtonBar& bar, juce::Graphics& g, int w, int h)
{
    g.setColour(juce::Colours::lightgrey);
    g.fillRect(bar.getLocalBounds());
}
