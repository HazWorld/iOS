#include "MainComponent.hpp"
#include "CustomLookAndFeel.hpp"

//==============================================================================
// MainComponent implementation
MainComponent::MainComponent()
    : tabs(juce::TabbedButtonBar::TabsAtBottom)
{
    DBG("MainComponent Constructor Called");


    setLookAndFeel(&customLookAndFeel);
    tabs.setLookAndFeel(&customLookAndFeel);

    tabs.setColour(juce::TabbedComponent::outlineColourId, juce::Colour::fromRGB(240, 230, 200));

    if (auto* display = juce::Desktop::getInstance().getDisplays().getPrimaryDisplay())
    {
        setBounds(display->userArea);
    }

    std::vector<std::tuple<juce::String, juce::Component*>> tabConfigs = {
        {"Chords", &tab1},
        {"Scales", &tab2},
        {"Tempo", &tab3}
    };


    addAndMakeVisible(tabs);


    for (const auto& config : tabConfigs)
    {
        tabs.addTab(std::get<0>(config), juce::Colours::transparentBlack, std::get<1>(config), false);
    }

    //Initialize audio
    setAudioChannels(1, 0);
}

MainComponent::~MainComponent()
{

    setLookAndFeel(nullptr);
    shutdownAudio();
}

void MainComponent::resized()
{
    auto bounds = getLocalBounds();


    const int topPadding = 44;
    const int bottomPadding = 34;

    bounds = bounds.withTrimmedTop(topPadding).withTrimmedBottom(bottomPadding);

    tabs.setBounds(bounds);
}

void MainComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour::fromRGB(240, 230, 200));
}

//audio preparation
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

    switch (tabs.getCurrentTabIndex())
    {
        case 1:
            tab2.processAudioBuffer(bufferToFill);
            break;

        case 2:
            tab3.processAudioBuffer(bufferToFill);
            break;

        default:
            bufferToFill.clearActiveBufferRegion();  //clears the buffer
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
    setAudioChannels(1, 0);
}
