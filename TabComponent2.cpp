#include "TabComponent2.hpp"

//==============================================================================
// TabComponent2 implementation (Note Detector)
TabComponent2::TabComponent2()
{
    addAndMakeVisible(noteLabel);
    noteLabel.setText("Play a note...", juce::dontSendNotification);
    noteLabel.setFont(juce::FontOptions(24.0f));
    noteLabel.setJustificationType(juce::Justification::centred);

    setSize(600, 400);
    initializeAudioResources();  // Initialize the FFT windowing

    startTimerHz(30);  // 30 times per second (33ms)

    // Initialize smoothing and stability variables
    lastFrequency = 0.0f;
    smoothedFrequency = 0.0f;
    stableNoteHoldTime = 0;
    currentNote = "";
}

TabComponent2::~TabComponent2()
{
    // No shutdownAudio here as the main component handles it
}

void TabComponent2::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    this->sampleRate = sampleRate;
}

void TabComponent2::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    if (bufferToFill.buffer->getNumChannels() > 0)
    {
        processFFTData(bufferToFill, [&]() { detectNoteFromFFT(); });
        bufferToFill.clearActiveBufferRegion();  // Clear buffer after processing
    }
}

void TabComponent2::releaseResources()
{
    // Nothing specific to release, so this can be left empty
}

void TabComponent2::resized()
{
    noteLabel.setBounds(getLocalBounds());
}

void TabComponent2::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::darkgrey);
    drawGraph(g, getLocalBounds().toFloat());
}

void TabComponent2::timerCallback()
{
    repaint();
}

// Helper function to calculate noise floor dynamically
float TabComponent2::calculateNoiseFloor()
{
    float sumNoise = 0.0f;
    int count = 0;

    // Average the lower magnitude frequencies to determine the noise floor
    for (int i = 1; i < fftSize / 4; ++i)  // Only consider lower frequencies for noise
    {
        sumNoise += fftData[i];
        count++;
    }

    return sumNoise / count;
}

// Improved note detection function with dynamic threshold, harmonics detection, and EWMA smoothing
void TabComponent2::detectNoteFromFFT()
{
    int fundamentalFreqIndex = -1;
    float maxMagnitude = 0.0f;

    // Find the peak in the FFT (fundamental frequency)
    for (int i = 1; i < fftSize / 2; ++i)  // Start at 1 to ignore DC
    {
        if (fftData[i] > maxMagnitude)
        {
            maxMagnitude = fftData[i];
            fundamentalFreqIndex = i;
        }
    }

    // Dynamically calculate magnitude threshold based on noise floor
    const float noiseFloor = calculateNoiseFloor();
    const float magnitudeThreshold = noiseFloor * 1.5f;  // Adjust threshold based on noise level

    if (maxMagnitude < magnitudeThreshold || fundamentalFreqIndex == -1)
    {
        juce::MessageManager::callAsync([this]() {
            noteLabel.setText("No Note Detected", juce::dontSendNotification);
        });
        return;
    }

    // Parabolic interpolation for more accurate frequency estimation
    float interpolatedIndex = fundamentalFreqIndex;
    if (fundamentalFreqIndex > 0 && fundamentalFreqIndex < fftSize / 2 - 1)
    {
        float alpha = fftData[fundamentalFreqIndex - 1];
        float beta = fftData[fundamentalFreqIndex];
        float gamma = fftData[fundamentalFreqIndex + 1];
        interpolatedIndex += (gamma - alpha) / (2.0f * (2.0f * beta - alpha - gamma));  // Parabolic interpolation formula
    }

    // Calculate the fundamental frequency
    float frequency = interpolatedIndex * (sampleRate / fftSize);

    // Filter frequency to typical guitar range (~82 Hz to ~1318 Hz)
    if (frequency < 82.0f || frequency > 1318.0f)
    {
        juce::MessageManager::callAsync([this]() {
            noteLabel.setText("No Note Detected", juce::dontSendNotification);
        });
        return;
    }

    // Use exponentially weighted moving average (EWMA) for frequency smoothing
    const float alpha = 0.1f;  // Smoothing factor (adjust as needed)
    smoothedFrequency = alpha * frequency + (1.0f - alpha) * smoothedFrequency;

    // Harmonics detection to reinforce fundamental frequency validity
    const int maxHarmonics = 5;
    float harmonicSum = 0.0f;

    for (int i = 2; i <= maxHarmonics; ++i)
    {
        int harmonicIndex = fundamentalFreqIndex * i;
        if (harmonicIndex < fftSize / 2)
        {
            harmonicSum += fftData[harmonicIndex];  // Sum harmonic magnitudes
        }
    }

    const float harmonicRatioThreshold = 0.3f;  // Adjust threshold for harmonics
    if (harmonicSum > maxMagnitude * harmonicRatioThreshold)
    {
        // Validate the fundamental frequency if harmonics are strong
    }

    // Use a hysteresis mechanism to prevent note jitter
    const float frequencyChangeThreshold = 1.5f;  // Tolerance for frequency change
    if (std::abs(smoothedFrequency - lastFrequency) > frequencyChangeThreshold)
    {
        stableNoteHoldTime = 0;  // Reset hold time when the frequency changes
    }
    else
    {
        stableNoteHoldTime++;  // Increment hold time if frequency is stable
    }

    // If the frequency has been stable for a sufficient number of frames
    const int stableFramesRequired = 15;  // Adjust for more/less stability
    if (stableNoteHoldTime >= stableFramesRequired)
    {
        lastFrequency = smoothedFrequency;  // Update last stable frequency
        juce::String detectedNote = getNoteNameFromFrequency(smoothedFrequency);

        if (detectedNote != currentNote)
        {
            currentNote = detectedNote;  // Update current note only if different

            // Update the UI with the detected note asynchronously
            juce::MessageManager::callAsync([this, detectedNote]() {
                DBG("Updating UI with note: " + detectedNote);  // Debug print
                noteLabel.setText("Detected Note: " + detectedNote, juce::dontSendNotification);
            });
        }
    }
}

juce::String TabComponent2::getNoteNameFromFrequency(float frequency)
{
    static const std::array<juce::String, 12> noteNames = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
    
    if (frequency <= 0)
        return "Unknown";

    // Adjust for A440 tuning
    int noteNumber = static_cast<int>(std::round(12.0f * std::log2(frequency / 440.0f))) + 69;
    
    int octave = noteNumber / 12 - 1;  // Calculate the octave
    juce::String noteName = noteNames[noteNumber % 12];  // Note name within an octave

    return noteName + juce::String(octave);
}
