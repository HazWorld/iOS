// MainComponent.cpp
#include "MainComponent.hpp"

//==============================================================================
// MainComponent implementation
MainComponent::MainComponent()
    : tabs(juce::TabbedButtonBar::TabsAtBottom)
{
    DBG("MainComponent Constructor Called");

    // Apply the custom LookAndFeel for iPhone-style design
    setLookAndFeel(&customLookAndFeel);  // Set custom LookAndFeel

    addAndMakeVisible(tabs);

    tabs.addTab("Chord Detector", juce::Colours::lightblue, &tab1, false);
    tabs.addTab("Note Detector", juce::Colours::lightgreen, &tab2, false);
    tabs.addTab("Settings", juce::Colours::lightcoral, &tab3, false);

    // Initialize audio with 1 input channel and no output
    setAudioChannels(1, 0);  // Mono input, no output

    setSize(600, 400);
}

MainComponent::~MainComponent()
{
    setLookAndFeel(nullptr);  // Reset LookAndFeel when destroying the component
    shutdownAudio();  // Clean up audio resources
}

void MainComponent::resized()
{
    auto bounds = getLocalBounds();
    int padding = 20;  // Padding for the layout
    bounds.reduce(padding, padding);  // Add padding around the edges

    tabs.setBounds(bounds);
}

void MainComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::white);  // Clean white background to match iOS style
}

//==============================================================================
// Audio setup and handling methods
void MainComponent::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    DBG("prepareToPlay called with sampleRate: " + juce::String(sampleRate) +
        " and samplesPerBlockExpected: " + juce::String(samplesPerBlockExpected));

    // Call prepareToPlay on the active tab (if necessary)
    tab1.prepareToPlay(samplesPerBlockExpected, sampleRate);
    tab2.prepareToPlay(samplesPerBlockExpected, sampleRate);
}

void MainComponent::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    if (bufferToFill.buffer == nullptr || bufferToFill.buffer->getNumChannels() == 0)
    {
        bufferToFill.clearActiveBufferRegion();
        return;
    }

    // Pass the buffer to the active tab only
//    if (tabs.getCurrentTabIndex() == 0)  // Chord Detector Tab
//        tab1.processAudioBuffer(bufferToFill);
    else if (tabs.getCurrentTabIndex() == 1)  // Note Detector Tab
        tab2.processAudioBuffer(bufferToFill);
    else
        bufferToFill.clearActiveBufferRegion();  // Clear buffer if not handled
}

void MainComponent::releaseResources()
{
    tab1.releaseResources();
//    tab2.releaseResources();
}
