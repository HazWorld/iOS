#pragma once

#include "JuceHeader.h"
#include "TabComponent1.hpp"
#include "TabComponent2.hpp"
#include "TabComponent3.hpp"
#include "CustomLookAndFeel.hpp"

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
    void suspended();
    void resumed();
    

private:
    juce::TabbedComponent tabs;
    TabComponent1 tab1;
    TabComponent2 tab2;
    TabComponent3 tab3;

    CustomLookAndFeel customLookAndFeel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
