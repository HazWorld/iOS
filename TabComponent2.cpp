#include "TabComponent2.hpp"

//==============================================================================
// TabComponent2 implementation (Note Detector)
TabComponent2::TabComponent2()
{
    addAndMakeVisible(noteLabel);
    noteLabel.setText("Play a note...", juce::dontSendNotification);
    noteLabel.setFont(juce::FontOptions(24.0f));  // Use Font for text
    noteLabel.setJustificationType(juce::Justification::centred);

    setSize(600, 400);

    // Initialize smoothing and stability variables
    lastFrequency = 0.0f;
    smoothedFrequency = 0.0f;
    stableNoteHoldTime = 0;
    currentNote = "";
}

TabComponent2::~TabComponent2() { }

void TabComponent2::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    yinProcessor.initialize(sampleRate, samplesPerBlockExpected);  // Initialize YIN

    // Initialize the low-pass filter to filter out higher frequencies
    lowPassFilter.setCoefficients(juce::IIRCoefficients::makeLowPass(sampleRate, 1500.0));
}

void TabComponent2::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    if (bufferToFill.buffer != nullptr && bufferToFill.buffer->getNumChannels() > 0)
    {
        const float* inputChannelData = bufferToFill.buffer->getReadPointer(0);
        int numSamples = bufferToFill.buffer->getNumSamples();

        if (numSamples > 0)
        {
            // Apply low-pass filter to remove high-frequency noise
            lowPassFilter.processSamples(bufferToFill.buffer->getWritePointer(0), numSamples);

            // Detect note from the filtered input
            detectNoteFromYIN(inputChannelData, numSamples);

            // Trigger a repaint to show the new waveform
            repaint();
        }
    }
}

void TabComponent2::detectNoteFromYIN(const float* audioBuffer, int numSamples)
{
    float detectedPitch = yinProcessor.process(audioBuffer, numSamples);

    if (detectedPitch > 0.0f)  // Valid pitch detected
    {
        const float alpha = 0.2f;  // Slightly higher smoothing factor for better response
        smoothedFrequency = alpha * detectedPitch + (1.0f - alpha) * smoothedFrequency;

        // Stability mechanism: update the UI only if the frequency is stable
        const float frequencyChangeThreshold = 0.5f;  // Tighter allowable deviation
        if (std::abs(smoothedFrequency - lastFrequency) > frequencyChangeThreshold)
        {
            stableNoteHoldTime = 0;  // Reset stability counter
        }
        else
        {
            stableNoteHoldTime++;  // Increment stability counter
        }

        const int stableFramesRequired = 8;  // Fewer frames for faster response
        if (stableNoteHoldTime >= stableFramesRequired)
        {
            lastFrequency = smoothedFrequency;  // Update the stable frequency
            juce::String detectedNote = getNoteNameFromFrequency(smoothedFrequency);
            updateNoteUI(detectedNote);
        }
    }
    else
    {
        juce::MessageManager::callAsync([this]() {
            noteLabel.setText("No Note Detected", juce::dontSendNotification);
        });
    }
}

juce::String TabComponent2::getNoteNameFromFrequency(float frequency)
{
    static const std::array<juce::String, 12> noteNames = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
    
    if (frequency <= 0)
        return "Unknown";

    int noteNumber = static_cast<int>(std::round(12.0f * std::log2(frequency / 440.0f))) + 69;
    int octave = noteNumber / 12 - 1;
    juce::String noteName = noteNames[noteNumber % 12];

    return noteName + juce::String(octave);
}

void TabComponent2::updateNoteUI(const juce::String& detectedNote)
{
    if (detectedNote != currentNote)
    {
        currentNote = detectedNote;

        juce::MessageManager::callAsync([this, detectedNote]() {
            noteLabel.setFont(juce::FontOptions(40.0f, juce::Font::bold));  // Font settings
            noteLabel.setColour(juce::Label::textColourId, juce::Colours::cyan);
            noteLabel.setText("Detected Note: " + detectedNote, juce::dontSendNotification);
        });
    }
}

// Paint method definition to fix the undefined symbol error
void TabComponent2::paint(juce::Graphics& g)
{
    // Fill the background with a color
    g.fillAll(juce::Colours::darkgrey);  // Dark grey background

    // Draw the waveform from YINAudioComponent
    const std::vector<float>& waveform = yinProcessor.getWaveformBuffer();
    if (!waveform.empty())
    {
        g.setColour(juce::Colours::cyan);  // Set waveform color
        juce::Path waveformPath;

        // Map the waveform values to the screen
        const int width = getWidth();
        const int height = getHeight();
        const float centerY = height * 0.5f;

        waveformPath.startNewSubPath(0, centerY);

        for (int i = 0; i < width && i < waveform.size(); ++i)
        {
            const float x = (float)i;
            const float y = centerY - waveform[i] * (centerY);  // Scale waveform vertically
            waveformPath.lineTo(x, y);
        }

        g.strokePath(waveformPath, juce::PathStrokeType(1.0f));  // Draw waveform with 1px stroke
    }

    // Set text properties and draw the title
    g.setFont(16.0f);
    g.setColour(juce::Colours::white);
    g.drawText("Note Detector", getLocalBounds(), juce::Justification::centredTop, true);  // Title at the top
}

void TabComponent2::resized()
{
    noteLabel.setBounds(getLocalBounds());
}
