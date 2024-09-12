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
        // Major chords (Root, Major Third, Perfect Fifth)
        {"C Major", {0, 4, 7}},    // C, E, G
        {"D Major", {2, 6, 9}},    // D, F#, A
        {"E Major", {4, 8, 11}},   // E, G#, B
        {"F Major", {5, 9, 0}},    // F, A, C
        {"G Major", {7, 11, 2}},   // G, B, D
        {"A Major", {9, 1, 4}},    // A, C#, E
        {"B Major", {11, 3, 6}},   // B, D#, F#

        // Minor chords (Root, Minor Third, Perfect Fifth)
        {"C Minor", {0, 3, 7}},    // C, Eb, G
        {"D Minor", {2, 5, 9}},    // D, F, A
        {"E Minor", {4, 7, 11}},   // E, G, B
        {"F Minor", {5, 8, 0}},    // F, Ab, C
        {"G Minor", {7, 10, 2}},   // G, Bb, D
        {"A Minor", {9, 0, 4}},    // A, C, E
        {"B Minor", {11, 2, 6}},   // B, D, F#

        // Diminished chords (Root, Minor Third, Diminished Fifth)
//        {"C Diminished", {0, 3, 6}},    // C, Eb, Gb
//        {"D Diminished", {2, 5, 8}},    // D, F, Ab
//        {"E Diminished", {4, 7, 10}},   // E, G, Bb
//        {"F Diminished", {5, 8, 11}},   // F, Ab, B
//        {"G Diminished", {7, 10, 1}},   // G, Bb, Db
//        {"A Diminished", {9, 0, 3}},    // A, C, Eb
//        {"B Diminished", {11, 2, 5}},   // B, D, F
//
//        // Dominant Seventh chords (Root, Major Third, Perfect Fifth, Minor Seventh)
//        {"C7", {0, 4, 7, 10}},    // C, E, G, Bb
//        {"D7", {2, 6, 9, 0}},     // D, F#, A, C
//        {"E7", {4, 8, 11, 2}},    // E, G#, B, D
//        {"F7", {5, 9, 0, 3}},     // F, A, C, Eb
//        {"G7", {7, 11, 2, 5}},    // G, B, D, F
//        {"A7", {9, 1, 4, 7}},     // A, C#, E, G
//        {"B7", {11, 3, 6, 9}},    // B, D#, F#, A
//
//        // Major Seventh chords (Root, Major Third, Perfect Fifth, Major Seventh)
//        {"C Major 7", {0, 4, 7, 11}},    // C, E, G, B
//        {"D Major 7", {2, 6, 9, 1}},     // D, F#, A, C#
//        {"E Major 7", {4, 8, 11, 3}},    // E, G#, B, D#
//        {"F Major 7", {5, 9, 0, 4}},     // F, A, C, E
//        {"G Major 7", {7, 11, 2, 6}},    // G, B, D, F#
//        {"A Major 7", {9, 1, 4, 8}},     // A, C#, E, G#
//        {"B Major 7", {11, 3, 6, 10}},   // B, D#, F#, A#
//
//        // Minor Seventh chords (Root, Minor Third, Perfect Fifth, Minor Seventh)
//        {"C Minor 7", {0, 3, 7, 10}},    // C, Eb, G, Bb
//        {"D Minor 7", {2, 5, 9, 0}},     // D, F, A, C
//        {"E Minor 7", {4, 7, 11, 2}},    // E, G, B, D
//        {"F Minor 7", {5, 8, 0, 3}},     // F, Ab, C, Eb
//        {"G Minor 7", {7, 10, 2, 5}},    // G, Bb, D, F
//        {"A Minor 7", {9, 0, 4, 7}},     // A, C, E, G
//        {"B Minor 7", {11, 2, 6, 9}},    // B, D, F#, A
//
//        // Augmented chords (Root, Major Third, Augmented Fifth)
//        {"C Augmented", {0, 4, 8}},    // C, E, G#
//        {"D Augmented", {2, 6, 10}},   // D, F#, A#
//        {"E Augmented", {4, 8, 0}},    // E, G#, C
//        {"F Augmented", {5, 9, 1}},    // F, A, C#
//        {"G Augmented", {7, 11, 3}},   // G, B, D#
//        {"A Augmented", {9, 1, 5}},    // A, C#, E#
//        {"B Augmented", {11, 3, 7}},   // B, D#, F##
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
    static std::array<float, 12> smoothedNoteMagnitudes = { 0 };  // Smoothing buffer for note magnitudes
    const float harmonicWeights[] = { 1.0f, 0.5f, 0.25f, 0.125f };
    const float smoothingFactor = 0.2f;
    const float binFrequencyStep = sampleRate / fftSize;
    const float noiseThreshold = 0.001f;

    // Minimum magnitude threshold to avoid detecting chords when no significant signal is present
    const float globalMinMagnitudeThreshold = 0.01f;

    std::array<float, 12> noteMagnitudes = { 0 };

    // Analyze FFT data and accumulate note magnitudes, including harmonics
    for (int i = 1; i < fftSize / 2; ++i)  // Start from i=1 to skip DC (0 Hz)
    {
        float frequency = i * binFrequencyStep;

        // Skip frequencies outside the typical guitar range (80 Hz to 1318 Hz)
        if (frequency < 80.0f || frequency > 1318.0f || fftData[i] < noiseThreshold)
            continue;

        // Parabolic interpolation for more accurate peak frequency detection
        if (i > 0 && i < fftSize / 2 - 1)
        {
            float alpha = fftData[i - 1];
            float beta = fftData[i];
            float gamma = fftData[i + 1];
            frequency += (gamma - alpha) / (2.0f * (2.0f * beta - alpha - gamma)) * binFrequencyStep;
        }

        // Calculate the note index relative to A440 (12-tone equal temperament)
        int noteIndex = static_cast<int>(std::round(12.0f * std::log2(frequency / 440.0f))) % 12;
        if (noteIndex < 0) noteIndex += 12;  // Wrap around negative values

        // Accumulate the magnitude of the fundamental frequency
        float magnitude = fftData[i];
        noteMagnitudes[noteIndex] += magnitude;

        // Add harmonic contributions with weighted magnitudes
        for (int harmonic = 2; harmonic <= 5; ++harmonic)
        {
            int harmonicIndex = (noteIndex * harmonic) % 12;
            noteMagnitudes[harmonicIndex] += (magnitude / harmonic) * harmonicWeights[harmonic - 2];
        }
    }

    // Smooth note magnitudes over time using exponential moving average (EMA)
    for (int i = 0; i < 12; ++i)
    {
        smoothedNoteMagnitudes[i] = (smoothingFactor * noteMagnitudes[i]) + ((1.0f - smoothingFactor) * smoothedNoteMagnitudes[i]);
    }

    // Find the maximum magnitude and normalize the smoothed note magnitudes
    float maxMagnitude = *std::max_element(smoothedNoteMagnitudes.begin(), smoothedNoteMagnitudes.end());

    // Check if the signal is too weak (likely no chord played)
    if (maxMagnitude < globalMinMagnitudeThreshold)
    {
        juce::MessageManager::callAsync([this]()
        {
            chordLabel.setText("No Chord Detected", juce::dontSendNotification);
        });
        return;
    }

    // Normalize the smoothed magnitudes
    if (maxMagnitude > 0.0f)
    {
        for (auto& magnitude : smoothedNoteMagnitudes)
        {
            magnitude /= maxMagnitude;
        }
    }

    const float threshold = maxMagnitude * 0.5f;

    // Chord detection logic with partial matches and weighted confidence
    juce::String detectedChord = "Unknown Chord";
    float bestMatchConfidence = 0.0f;

    for (const auto& [chordName, templateNotes] : chordTemplates)
    {
        int matchedNotes = 0;
        float confidence = 0.0f;

        for (int note : templateNotes)
        {
            if (smoothedNoteMagnitudes[note] >= threshold)
            {
                matchedNotes++;
                confidence += smoothedNoteMagnitudes[note];
            }
        }

        // Partial matching: We can tolerate a missing note or two, but prioritize strong matches
        float matchConfidence = confidence / templateNotes.size();
        if (matchedNotes >= (templateNotes.size() - 1) && matchConfidence > bestMatchConfidence)  // Allow 1 missing note
        {
            detectedChord = chordName;
            bestMatchConfidence = matchConfidence;
        }
    }

    // Chord stability mechanism
    static juce::String lastDetectedChord = "Unknown Chord";
    static int stableFrameCount = 0;
    const int stableFramesRequired = 10;

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
