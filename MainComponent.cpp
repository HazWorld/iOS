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

    // Define tab configurations: Title, Color, and Component
    std::vector<std::tuple<juce::String, juce::Colour, juce::Component*>> tabConfigs = {
           {"Chord Detector", juce::Colours::lightblue, &tab1},
           {"Scales", juce::Colours::lightgreen, &tab2},
           {"Tempo", juce::Colours::lightcoral, &tab3}
       };
       
       addAndMakeVisible(tabs);

       // Add tabs dynamically
       for (const auto& config : tabConfigs)
       {
           tabs.addTab(std::get<0>(config), std::get<1>(config), std::get<2>(config), false);
       }

       setAudioChannels(1, 0);  // Mono input, no output
}

MainComponent::~MainComponent()
{
    setLookAndFeel(nullptr);  // Reset LookAndFeel when destroying the component
    shutdownAudio();  // Clean up audio resources
}

void MainComponent::resized()
{
    auto bounds = getLocalBounds();

  
    int padding = bounds.getWidth() / 25;
    bounds.reduce(padding, padding);

  
    tabs.setBounds(bounds.withTrimmedBottom(20));
}

void MainComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour::fromRGB(240, 230, 200));  // Clean white background to match iOS style
}


// Audio preparation
void MainComponent::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    DBG("prepareToPlay called with sampleRate: " + juce::String(sampleRate) +
        " and samplesPerBlockExpected: " + juce::String(samplesPerBlockExpected));

    tab2.prepareToPlay(samplesPerBlockExpected, sampleRate);
}

void MainComponent::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    if (bufferToFill.buffer == nullptr || bufferToFill.buffer->getNumChannels() == 0)
    {
        bufferToFill.clearActiveBufferRegion();
        return;
    }
    // Process audio based on the selected tab
    switch (tabs.getCurrentTabIndex())
    {
        case 1:  // Scales
            tab2.processAudioBuffer(bufferToFill);
            break;

        case 2:  // Tempo
            tab3.processAudioBuffer(bufferToFill);
            break;

        default:
            bufferToFill.clearActiveBufferRegion();  // Clears the buffer
            break;
    }
}

void MainComponent::releaseResources()
{
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
