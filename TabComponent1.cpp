#include "TabComponent1.hpp"

//==============================================================================
// TabComponent1 implementation (Chord Detector)
TabComponent1::TabComponent1()
{
    // Set up chord label
    addAndMakeVisible(chordLabel);
    chordLabel.setText("Play a chord...", juce::dontSendNotification);
    chordLabel.setFont(juce::FontOptions(32.0f, juce::Font::bold));
    chordLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    chordLabel.setJustificationType(juce::Justification::centred);

    setSize(600, 400);

    // Initialize chord templates
    chordTemplates = {
        {"C Major", {0, 4, 7}}, {"D Major", {2, 6, 9}}, {"E Major", {4, 8, 11}},
        {"F Major", {5, 9, 0}}, {"G Major", {7, 11, 2}}, {"A Major", {9, 1, 4}},
        {"B Major", {11, 3, 6}}, {"C Minor", {0, 3, 7}}, {"D Minor", {2, 5, 9}},
        {"E Minor", {4, 7, 11}}, {"F Minor", {5, 8, 0}}, {"G Minor", {7, 10, 2}},
        {"A Minor", {9, 0, 4}}, {"B Minor", {11, 2, 6}}
    };
}

TabComponent1::~TabComponent1()
{
}

void TabComponent1::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    DBG("prepared tab1 at : " + juce::String(samplesPerBlockExpected) + " and : " + juce::String(sampleRate));
    yinComponent.initialize(sampleRate, samplesPerBlockExpected);
}


void TabComponent1::processAudioBuffer(const juce::AudioSourceChannelInfo& bufferToFill)
{
    if (bufferToFill.buffer != nullptr && bufferToFill.buffer->getNumChannels() > 0)
    {
//        DBG("processing audio tab 1");
        for (int sample = 0; sample < bufferToFill.buffer->getNumSamples(); ++sample)
        {
            float sum = 0.0f;
            for (int channel = 0; channel < bufferToFill.buffer->getNumChannels(); ++channel)
                sum += bufferToFill.buffer->getReadPointer(channel)[sample];

            float monoSample = sum / bufferToFill.buffer->getNumChannels();

            // Use multiple pitch detection
            auto detectedPitches = yinComponent.processMultiplePitches(&monoSample, 1);

            if (!detectedPitches.empty())
            {
                juce::MessageManager::callAsync([this, detectedPitches]()
                {
                    detectChordFromPitches(detectedPitches);
                });
            }
        }
    }
}

void TabComponent1::releaseResources()
{
    // No resources to release currently
}

void TabComponent1::resized()
{
    // FlexBox layout for centralized, responsive UI
    juce::FlexBox flexbox;
    flexbox.flexDirection = juce::FlexBox::Direction::column;
    flexbox.justifyContent = juce::FlexBox::JustifyContent::center;
    flexbox.alignItems = juce::FlexBox::AlignItems::center;

    flexbox.items.add(juce::FlexItem(chordLabel).withWidth(300.0f).withHeight(60.0f).withMargin(juce::FlexItem::Margin(10.0f)));
    flexbox.performLayout(getLocalBounds());
}

void TabComponent1::paint(juce::Graphics& g)
{
    juce::ColourGradient gradient(juce::Colours::darkblue, getWidth() / 2, getHeight() / 2,
                                  juce::Colours::midnightblue, getWidth(), getHeight(), false);
    g.setGradientFill(gradient);
    g.fillAll();

    g.setFont(juce::FontOptions(16.0f));
    g.setColour(juce::Colours::lightgrey);
    g.drawText("Guitar Chord Detector", getLocalBounds().reduced(10), juce::Justification::centredTop, true);
}



void TabComponent1::detectChordFromPitches(const std::vector<float>& detectedPitches)
{
    static std::array<float, 12> smoothedNoteMagnitudes = { 0 };
    const float smoothingFactor = 0.2f;
    const float noteDetectionThreshold = 0.7f;

    std::array<float, 12> noteMagnitudes = { 0 };
    
    for (float pitch : detectedPitches)
    {
        int noteIndex = static_cast<int>(std::round(12.0f * std::log2(pitch / 440.0f))) % 12;
        if (noteIndex < 0) noteIndex += 12;

        noteMagnitudes[noteIndex] = 1.0f;

        // Reduce harmonics
        int harmonicIndex = (noteIndex + 7) % 12;
        noteMagnitudes[harmonicIndex] *= 0.5f;
    }

    // Smooth the note magnitudes
    std::transform(smoothedNoteMagnitudes.begin(), smoothedNoteMagnitudes.end(), noteMagnitudes.begin(),
                   smoothedNoteMagnitudes.begin(),
                   [smoothingFactor](float smoothed, float current)
                   {
                       return (smoothingFactor * current) + ((1.0f - smoothingFactor) * smoothed);
                   });

    // Existing chord detection logic (slightly updated to handle multiple detected pitches)
    juce::String detectedChord = "Unknown Chord";
    float bestMatchConfidence = 0.0f;

    for (const auto& [chordName, templateNotes] : chordTemplates)
    {
        int matchedNotes = 0;
        float confidence = 0.0f;

        for (int note : templateNotes)
        {
            if (smoothedNoteMagnitudes[note] > noteDetectionThreshold)
            {
                matchedNotes++;
                confidence += smoothedNoteMagnitudes[note];
            }
        }

        if (matchedNotes >= (templateNotes.size() - 1) && confidence > bestMatchConfidence)
        {
            detectedChord = chordName;
            bestMatchConfidence = confidence;
        }
    }

    static juce::String lastDetectedChord = "Unknown Chord";
    static int stableFrameCount = 0;

    if (detectedChord == lastDetectedChord)
    {
        stableFrameCount++;
    }
    else
    {
        stableFrameCount = 0;
        lastDetectedChord = detectedChord;
    }

    if (stableFrameCount >= stableFramesRequired)
    {
        juce::MessageManager::callAsync([this, detectedChord, bestMatchConfidence]()
        {
            if (bestMatchConfidence > 0.8f)
                chordLabel.setColour(juce::Label::textColourId, juce::Colours::limegreen);
            else if (bestMatchConfidence > 0.5f)
                chordLabel.setColour(juce::Label::textColourId, juce::Colours::yellow);
            else
                chordLabel.setColour(juce::Label::textColourId, juce::Colours::red);

            chordLabel.setText("Detected Chord: " + detectedChord + " (Confidence: " + juce::String(bestMatchConfidence * 100.0f, 1) + "%)", juce::dontSendNotification);
        });
    }
}
