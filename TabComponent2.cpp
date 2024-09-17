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
    lastFrequency = 0.0f;
    currentNote = "";

    startTimerHz(60);  // Timer for UI responsiveness
}

TabComponent2::~TabComponent2()
{
    stopTimer();
}

void TabComponent2::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    yinProcessor.initialize(sampleRate, samplesPerBlockExpected);
}

void TabComponent2::processAudioBuffer(const juce::AudioSourceChannelInfo& bufferToFill)
{
    if (bufferToFill.buffer != nullptr && bufferToFill.buffer->getNumChannels() > 0)
    {
        // Convert stereo to mono by averaging the channels
        juce::AudioBuffer<float> tempBuffer(1, bufferToFill.buffer->getNumSamples());
        for (int sample = 0; sample < tempBuffer.getNumSamples(); ++sample)
        {
            float sum = 0.0f;
            for (int channel = 0; channel < bufferToFill.buffer->getNumChannels(); ++channel)
                sum += bufferToFill.buffer->getReadPointer(channel)[sample];

            tempBuffer.setSample(0, sample, sum / bufferToFill.buffer->getNumChannels());
        }

        // Get the detected pitch from the YIN processor
        float detectedPitch = yinProcessor.processAudioBuffer(tempBuffer.getReadPointer(0), tempBuffer.getNumSamples());

        if (detectedPitch > 0.0f)
            updateNoteUI(getNoteNameFromFrequencyWithTolerance(detectedPitch));

        // Update waveform visualization buffer
        waveformBuffer = yinProcessor.getVisualizationData();
    }
}

juce::String TabComponent2::getNoteNameFromFrequencyWithTolerance(float frequency)
{
    static const std::array<juce::String, 12> noteNames = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };

    // Define the tolerance for note detection (3Hz above/below the exact note frequency)
    const float tolerance = 3.0f;

    // Convert frequency to a MIDI note number
    int noteNumber = static_cast<int>(std::round(12.0f * std::log2(frequency / 440.0f))) + 69;
    int octave = noteNumber / 12 - 1;
    juce::String noteName = noteNames[noteNumber % 12];

    // Calculate the frequency of the nearest note
    float closestFrequency = 440.0f * std::pow(2.0f, (noteNumber - 69) / 12.0f);

    // Only return the note name if the detected frequency is within the tolerance range
    if (std::abs(frequency - closestFrequency) <= tolerance)
    {
        return noteName + juce::String(octave);  // e.g., "A4"
    }
    else
    {
        return "Unknown";  // Return "Unknown" if the frequency is outside the tolerance
    }
}

void TabComponent2::updateNoteUI(const juce::String& detectedNote)
{
    if (detectedNote != currentNote)
    {
        currentNote = detectedNote;

        juce::MessageManager::callAsync([this, detectedNote]()
        {
            noteLabel.setText("Detected Note: " + detectedNote, juce::dontSendNotification);
        });
    }
}

void TabComponent2::timerCallback()
{
    repaint();  // Redraw UI at regular intervals
}

void TabComponent2::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::darkgrey);

    // Draw the waveform if data is available
    if (!waveformBuffer.empty())
    {
        g.setColour(juce::Colours::cyan);
        juce::Path waveformPath;
        const int width = getWidth();
        const int height = getHeight();
        const float centerY = height * 0.5f;

        waveformPath.startNewSubPath(0, centerY);

        for (int i = 0; i < width && i < waveformBuffer.size(); ++i)
        {
            const float x = (float)i;
            const float y = centerY - waveformBuffer[i] * centerY;
            waveformPath.lineTo(x, y);
        }

        g.strokePath(waveformPath, juce::PathStrokeType(1.0f));
    }

    g.setFont(16.0f);
    g.setColour(juce::Colours::white);
    g.drawText("Note Detector", getLocalBounds(), juce::Justification::centredTop, true);
}

void TabComponent2::resized()
{
    noteLabel.setBounds(getLocalBounds());
}
