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

// Improved note detection function with frequency smoothing and note stability
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

    // Apply a minimum magnitude threshold to avoid detecting noise
    const float magnitudeThreshold = 0.01f;  // Adjust this as needed
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

    // Smoothing frequency over multiple frames
    const float smoothingFactor = 0.2f;  // Adjust for more/less smoothing
    smoothedFrequency = (smoothingFactor * frequency) + ((1.0f - smoothingFactor) * smoothedFrequency);

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
