#include "TabComponent1.hpp"

//==============================================================================
// TabComponent1 implementation (Chord Detector)
TabComponent1::TabComponent1()
{
    addAndMakeVisible(chordLabel);
    chordLabel.setText("Play a chord...", juce::dontSendNotification);
    chordLabel.setFont(juce::FontOptions(24.0f));
    chordLabel.setJustificationType(juce::Justification::centred);

    setSize(600, 400);
    initializeAudioResources();  // Initialize the FFT windowing

    startTimerHz(30);  // 30 times per second (33ms)
    
    chordTemplates = {
            {"C Major", {0, 4, 7}},   // C, E, G
            {"D Minor", {2, 5, 9}},   // D, F, A
            {"E Major", {4, 8, 11}},  // E, G#, B
            {"G Major", {7, 11, 2}},  // G, B, D
    };
}

TabComponent1::~TabComponent1()
{
    // No shutdownAudio here as the main component handles it
}

void TabComponent1::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    this->sampleRate = sampleRate;
}

void TabComponent1::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    processFFTData(bufferToFill, [&]() { detectChordFromFFT(); });
}

void TabComponent1::releaseResources()
{
    // Nothing specific to release, so this can be left empty
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

    drawGraph(g, getLocalBounds().toFloat());
}

void TabComponent1::timerCallback()
{
    repaint();
}

void TabComponent1::detectChordFromFFT()
{
    std::array<float, 12> noteMagnitudes = { 0 };
    float harmonicWeights[] = { 1.0f, 0.5f, 0.25f, 0.125f };

    // Analyze the FFT data and accumulate note magnitudes, including harmonics
    for (int i = 0; i < fftSize / 2; ++i)
    {
        float frequency = i * (sampleRate / fftSize);

        // Focus on the typical guitar range and exclude noise outside this range
        if (frequency > 80.0f && frequency < 5000.0f)  // Typical guitar frequency range
        {
            // Calculate the note index relative to A440 (12-tone equal temperament)
            int noteIndex = static_cast<int>(std::round(12.0f * std::log2(frequency / 440.0f))) % 12;
            if (noteIndex < 0) noteIndex += 12;

            // Accumulate the magnitude of the fundamental frequency
            noteMagnitudes[noteIndex] += fftData[i];

            // Add harmonics with weighted contributions
            for (int harmonic = 2; harmonic <= 5; ++harmonic)
            {
                int harmonicIndex = (noteIndex * harmonic) % 12;
                noteMagnitudes[harmonicIndex] += (fftData[i] / harmonic) * harmonicWeights[harmonic - 2];
            }
        }
    }

    // Normalize the magnitudes to get relative values between 0 and 1
    float totalMagnitude = std::accumulate(noteMagnitudes.begin(), noteMagnitudes.end(), 0.0f);
    if (totalMagnitude > 0.0f)
    {
        for (auto& magnitude : noteMagnitudes)
            magnitude /= totalMagnitude;  // Normalize to a range of 0.0 to 1.0
    }

    // Set a dynamic threshold based on the average magnitude, instead of max magnitude
    float averageMagnitude = totalMagnitude / noteMagnitudes.size();
    float threshold = averageMagnitude * 0.8f;  // Use 80% of the average magnitude as threshold

    juce::String detectedChord = "Unknown Chord";
    float bestMatchConfidence = 0.0f;  // Track the best match confidence score

    // Iterate through chord templates and try to match the detected notes
    for (const auto& [chordName, templateNotes] : chordTemplates)
    {
        int matchedNotes = 0;
        float confidence = 0.0f;

        for (int note : templateNotes)
        {
            if (noteMagnitudes[note] >= threshold)
            {
                matchedNotes++;
                confidence += noteMagnitudes[note];  // Accumulate confidence score
            }
        }

        // Calculate confidence score as a percentage of matched notes
        float matchConfidence = confidence / templateNotes.size();

        // Only consider it a match if all notes in the template are present
        if (matchedNotes == templateNotes.size() && matchConfidence > bestMatchConfidence)
        {
            detectedChord = chordName;
            bestMatchConfidence = matchConfidence;
        }
    }

    // Update the UI with the detected chord asynchronously
    juce::MessageManager::callAsync([this, detectedChord, bestMatchConfidence]()
    {
        chordLabel.setText("Detected Chord: " + detectedChord + " (Confidence: " + juce::String(bestMatchConfidence * 100.0f, 1) + "%)", juce::dontSendNotification);
    });
}
