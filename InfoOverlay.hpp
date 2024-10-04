#pragma once

#include "JuceHeader.h"

class InfoOverlay : public juce::Component
{
public:
    InfoOverlay()
    {
        addAndMakeVisible(infoLabel);
        infoLabel.setJustificationType(juce::Justification::centred);
        infoLabel.setFont(juce::FontOptions(18.0f));

        addAndMakeVisible(closeButton);
        closeButton.setButtonText("Exit");
        closeButton.onClick = [this]() { setVisible(false); };
        
    }

    void setInfoContent(const juce::String& infoText)
    {
        infoLabel.setText(infoText, juce::dontSendNotification);
    }

    void resized() override
    {
        auto area = getLocalBounds().reduced(20);
        infoLabel.setBounds(area.removeFromTop(area.getHeight() - 50));
        closeButton.setBounds(area.removeFromTop(30));
    }
    
    void paint(juce::Graphics& g) override
    {

        g.fillAll(juce::Colour(0, 0, 0).withAlpha(1.0f));
        g.setColour(juce::Colours::white);
        g.drawRect(getLocalBounds(), 2);
    }

private:
    juce::Label infoLabel;
    juce::TextButton closeButton;
};
