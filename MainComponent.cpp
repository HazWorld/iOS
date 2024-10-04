#include "MainComponent.hpp"
#include "CustomLookAndFeel.hpp"


//MainComponent
MainComponent::MainComponent()
    : tabs(juce::TabbedButtonBar::TabsAtBottom)
{
    DBG("MainComponent Constructor Called");

    // sets look and feel to tabs
    tabs.setLookAndFeel(&customLookAndFeel);

    tabs.setColour(juce::TabbedComponent::outlineColourId, juce::Colour::fromRGB(240, 230, 200));

    if (auto* display = juce::Desktop::getInstance().getDisplays().getPrimaryDisplay())
    {
        setBounds(display->userArea);
    }

    //setting tabs
    std::vector<std::tuple<juce::String, juce::Component*>> tabConfigs = {
        {"Chords", &tab1},
        {"Scales", &tab2},
        {"Tempo", &tab3}
    };


    addAndMakeVisible(tabs);

    //adds tabs
    for (const auto& config : tabConfigs)
    {
        tabs.addTab(std::get<0>(config), juce::Colours::transparentBlack, std::get<1>(config), false);
    }

    //Initialize audio
    setAudioChannels(1, 0);
}

//MainComponent destructor
MainComponent::~MainComponent()
{
    tabs.setLookAndFeel(nullptr);
    setLookAndFeel(nullptr);
    shutdownAudio();
}

//sets padding for safe area bounds
void MainComponent::resized()
{
    auto bounds = getLocalBounds();


    const int topPadding = 44;
    const int bottomPadding = 34;

    bounds = bounds.withTrimmedTop(topPadding).withTrimmedBottom(bottomPadding);

    tabs.setBounds(bounds);
}

//sets overall background
void MainComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour::fromRGB(240, 230, 200));
}

//audio preparation
void MainComponent::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    DBG("prepareToPlay called with sampleRate: " + juce::String(sampleRate) +
        " and samplesPerBlockExpected: " + juce::String(samplesPerBlockExpected));

    //calls prepare to play for tab 2 and 3
    tab2.prepareToPlay(samplesPerBlockExpected, sampleRate);
    tab3.prepareToPlay(samplesPerBlockExpected, sampleRate);
}

//this handles the audio buffer management depending on the selected tab
void MainComponent::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    if (bufferToFill.buffer == nullptr || bufferToFill.buffer->getNumChannels() == 0)
    {
        bufferToFill.clearActiveBufferRegion();
        return;
    }

    //process audio depending on selected tab
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

//calls release resources for tab 2 and 3
void MainComponent::releaseResources()
{
    tab2.releaseResources();
    tab3.releaseResources();
}

//handles app suspensions
void MainComponent::suspended()
{
    shutdownAudio();
}

//handles app resumes
void MainComponent::resumed()
{
    setAudioChannels(1, 0);
}
