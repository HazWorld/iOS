#include "CustomLookAndFeel.hpp"

CustomLookAndFeel::CustomLookAndFeel()
{
    
}

//this handles the dawing of the tab button
void CustomLookAndFeel::drawTabButton(juce::TabBarButton& button, juce::Graphics& g, bool isMouseOver, bool isMouseDown)
{
    auto area = button.getLocalBounds().toFloat();
    float cornerSize = 10.0f;


    juce::Colour tabBackground = button.isFrontTab()
        ? juce::Colour::fromRGB(186, 202, 169)  //green for selected
        : juce::Colours::lightgrey;  //grey for unselected

    
    g.setColour(tabBackground);
    g.fillRoundedRectangle(area, cornerSize);

    //sets tab colour depending on selection
    juce::Colour textColor = button.isFrontTab() ? juce::Colours::white : juce::Colours::darkgrey;
    g.setColour(textColor);

    //sets text boldness
    g.setFont(button.isFrontTab() ? juce::FontOptions(18.0f, juce::Font::bold) : juce::FontOptions(16.0f));

    g.drawText(button.getButtonText(), area.reduced(10), juce::Justification::centred, true);
}

int CustomLookAndFeel::getTabButtonBestWidth(juce::TabBarButton&, int)
{
    return 150; 
}

