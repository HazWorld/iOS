#include "TabComponent1.hpp"

//==============================================================================
// TabComponent1 implementation (Chord Detector)
TabComponent1::TabComponent1() : yinComponent(), sampleRate(0.0)
{
    addAndMakeVisible(chordLabel);
    chordLabel.setText("Play a chord...", juce::dontSendNotification);
    chordLabel.setFont(juce::FontOptions(24.0f));  // Use Font instead of FontOptions
    chordLabel.setJustificationType(juce::Justification::centred);

    setSize(600, 400);

    // Chord templates initialization
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
    this->sampleRate = sampleRate;
    yinComponent.initialize(sampleRate, samplesPerBlockExpected);  // Initialize YIN
}

void TabComponent1::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    // Ensure valid buffer
    if (bufferToFill.buffer == nullptr || bufferToFill.buffer->getNumChannels() == 0)
        return;

    const float* audioData = bufferToFill.buffer->getReadPointer(0, bufferToFill.startSample);
    int bufferSize = bufferToFill.numSamples;

    // Detect pitch using YIN
    float detectedPitch = yinComponent.process(audioData, bufferSize);

    if (detectedPitch > 0)
    {
        detectChordFromYIN(detectedPitch);
    }
}

void TabComponent1::releaseResources()
{
    // Nothing to release here anymore
}

void TabComponent1::resized()
{
    chordLabel.setBounds(getLocalBounds());
}

void TabComponent1::paint(juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
    g.setFont(16.0f);
    g.setColour(juce::Colours::white);
    g.drawText("Guitar Chord Detector", getLocalBounds(), juce::Justification::centredTop, true);
}

void TabComponent1::detectChordFromYIN(float detectedPitch)
{
    static std::array<float, 12> smoothedNoteMagnitudes = { 0 };  // Smoothed magnitudes
    const float smoothingFactor = 0.2f;

    // Map detected pitch to note index relative to A440
    int noteIndex = static_cast<int>(std::round(12.0f * std::log2(detectedPitch / 440.0f))) % 12;
    if (noteIndex < 0) noteIndex += 12;  // Wrap negative values

    // Simplified: assume detected pitch has full magnitude
    std::array<float, 12> noteMagnitudes = { 0 };
    noteMagnitudes[noteIndex] = 1.0f;

    // Smooth note magnitudes using exponential moving average (EMA)
    std::transform(smoothedNoteMagnitudes.begin(), smoothedNoteMagnitudes.end(), noteMagnitudes.begin(),
                   smoothedNoteMagnitudes.begin(),
                   [smoothingFactor](float smoothed, float current)
                   {
                       return (smoothingFactor * current) + ((1.0f - smoothingFactor) * smoothed);
                   });

    // Chord detection logic
    juce::String detectedChord = "Unknown Chord";
    float bestMatchConfidence = 0.0f;

    for (const auto& [chordName, templateNotes] : chordTemplates)
    {
        int matchedNotes = 0;
        float confidence = 0.0f;

        for (int note : templateNotes)
        {
            if (smoothedNoteMagnitudes[note] > 0.5f)
            {
                matchedNotes++;
                confidence += smoothedNoteMagnitudes[note];
            }
        }

        if (matchedNotes >= (templateNotes.size() - 1) && confidence > bestMatchConfidence)  // Allow 1 missing note
        {
            detectedChord = chordName;
            bestMatchConfidence = confidence;
        }
    }

    // Chord stability mechanism
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
            chordLabel.setText("Detected Chord: " + detectedChord + " (Confidence: " + juce::String(bestMatchConfidence * 100.0f, 1) + "%)", juce::dontSendNotification);
        });
    }
}
