#include "MainComponent.hpp"
#include "CustomLookAndFeel.hpp"

//==============================================================================
// MainComponent implementation
MainComponent::MainComponent()
    : tabs(juce::TabbedButtonBar::TabsAtBottom)
{
    DBG("MainComponent Constructor Called");

    // Apply the custom LookAndFeel for iPhone-style design
    setLookAndFeel(&customLookAndFeel);

    // Set full-screen behavior for mobile devices
    if (auto* display = juce::Desktop::getInstance().getDisplays().getPrimaryDisplay())
    {
        setBounds(display->userArea);  // Set the component to fill the entire screen
    }

    addAndMakeVisible(tabs);

    // Add tabs for different sections
    tabs.addTab("Chord Detector", juce::Colours::lightblue, &tab1, false);
    tabs.addTab("Scales", juce::Colours::lightgreen, &tab2, false);
    tabs.addTab("Tempo", juce::Colours::lightcoral, &tab3, false);

    // Initialize audio with mono input and no output
    setAudioChannels(1, 0);
}

MainComponent::~MainComponent()
{
    setLookAndFeel(nullptr);  // Reset LookAndFeel when destroying the component
    shutdownAudio();  // Clean up audio resources
}

void MainComponent::resized()
{
    auto bounds = getLocalBounds();
    int padding = bounds.getWidth() / 30;  // Set padding relative to screen width
    bounds.reduce(padding, padding);  // Add dynamic padding

    tabs.setBounds(bounds);
}

void MainComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::white);  // Clean white background to match iOS style
    g.setColour(juce::Colours::lightgrey);  // Light grey border
    g.drawRoundedRectangle(getLocalBounds().toFloat(), 20.0f, 2.0f);  // Rounded corners
}

//==============================================================================
void MainComponent::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    DBG("prepareToPlay called with sampleRate: " + juce::String(sampleRate) +
        " and samplesPerBlockExpected: " + juce::String(samplesPerBlockExpected));

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
    if (tabs.getCurrentTabIndex() == 0)  // Chord Detector
    {
//        DBG("Processing audio in Tab 1 (Chord Detector)");
        tab1.processAudioBuffer(bufferToFill);
    }
    else if (tabs.getCurrentTabIndex() == 1)  // Scales
    {
//        DBG("Processing audio in Tab 2 (Scales)");
        tab2.processAudioBuffer(bufferToFill);
    }
    else if (tabs.getCurrentTabIndex() == 2)  // Tempo
    {
//        DBG("Processing audio in Tab 3 (Tempo)");
        tab3.processAudioBuffer(bufferToFill);
    }
    else
    {
        bufferToFill.clearActiveBufferRegion();  // Clear buffer if not handled
    }
}

void MainComponent::releaseResources()
{
//    tab1.releaseResources();
    tab2.releaseResources();
    tab3.releaseResources();
}

void MainComponent::suspended()
{
    shutdownAudio();
}

void MainComponent::resumed()
{
    setAudioChannels(1, 0);  // Re-enable mono input, no output
}
