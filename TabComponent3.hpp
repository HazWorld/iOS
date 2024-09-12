
//==============================================================================
// Tab 2 header (Note Detector)
#pragma once

#include <JuceHeader.h>
#include <array>
#include <vector>
#include "FFTAudioComponent.hpp"

//==============================================================================
// Tab 3 header (Placeholder Content)
class TabComponent3 : public juce::Component
{
public:
    TabComponent3()
    {
        addAndMakeVisible(label);
        label.setText("Tab 3 Content", juce::dontSendNotification);
        label.setJustificationType(juce::Justification::centred);
    }

    void resized() override
    {
        label.setBounds(getLocalBounds());
    }

private:
    juce::Label label;
};
