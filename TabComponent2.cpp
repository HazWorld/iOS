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
    smoothedFrequency = 0.0f;
    stableNoteHoldTime = 0;
    currentNote = "";

    // Start a timer to control repainting frequency (60 frames per second for better responsiveness)
    startTimerHz(60);  // Increased frequency for more responsive UI
}

TabComponent2::~TabComponent2()
{
    stopTimer();
}

void TabComponent2::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    // Double the buffer size to improve low-frequency detection
    yinProcessor.initialize(sampleRate, samplesPerBlockExpected);
    lowPassFilter.setCoefficients(juce::IIRCoefficients::makeLowPass(sampleRate, 3000.0));
}

void TabComponent2::processAudioBuffer(const juce::AudioSourceChannelInfo& bufferToFill)
{
//    DBG("Processing audio buffer in TabComponent2");

    if (bufferToFill.buffer != nullptr && bufferToFill.buffer->getNumChannels() > 0)
    {
        int numSamples = bufferToFill.buffer->getNumSamples();
        int numChannels = bufferToFill.buffer->getNumChannels();

        if (numSamples > 0)
        {
            juce::AudioBuffer<float> tempBuffer(1, numSamples);  // Create a temporary buffer for mono data

            for (int sample = 0; sample < numSamples; ++sample)
            {
                float sum = 0.0f;
                for (int channel = 0; channel < numChannels; ++channel)
                {
                    sum += bufferToFill.buffer->getReadPointer(channel)[sample];
                }
                tempBuffer.setSample(0, sample, sum / numChannels);  // Convert to mono by averaging the channels
            }

            // Apply low-pass filter to remove high-frequency noise
            lowPassFilter.processSamples(tempBuffer.getWritePointer(0), numSamples);

            // Detect note from the filtered mono input (YIN now handles buffer accumulation)
            yinProcessor.processAudioBuffer(tempBuffer.getReadPointer(0), numSamples);
        }
    }
}

void TabComponent2::detectNoteFromYIN(const float* audioBuffer, int numSamples)
{
    DBG("Detect note method called");

    // Calculate the energy of the signal
    float energy = 0.0f;
    for (int i = 0; i < numSamples; ++i)
    {
        energy += audioBuffer[i] * audioBuffer[i];  // Sum of squares for RMS energy
    }

    // Lower the energy threshold to allow quieter notes
    if (energy < 1e-6f)  // Reduced from 1e-5f to detect quieter notes
    {
        DBG("Signal energy too low, skipping pitch detection.");
        return;  // Skip pitch detection if the energy is too low
    }

    float detectedPitch = yinProcessor.process(audioBuffer, numSamples);

    if (detectedPitch > 0.0f)  // Valid pitch detected
    {
        DBG("Detected pitch: " + juce::String(detectedPitch) + " Hz");

        // Apply exponential moving average for smoothing frequency
        const float alpha = 0.1f;  // Smoother transitions
        smoothedFrequency = alpha * detectedPitch + (1.0f - alpha) * smoothedFrequency;

        if (std::abs(smoothedFrequency - lastFrequency) > 0.5f)
        {
            stableNoteHoldTime = 0;
        }
        else
        {
            stableNoteHoldTime++;
        }

        if (stableNoteHoldTime >= 6)  // Stability increased slightly
        {
            lastFrequency = smoothedFrequency;
            juce::String detectedNote = getNoteNameFromFrequencyWithTolerance(smoothedFrequency);
            updateNoteUI(detectedNote);  // Update the UI with the detected note
        }
    }
    else
    {
        juce::MessageManager::callAsync([this]() {
            noteLabel.setText("No Note Detected", juce::dontSendNotification);
        });
    }
}

juce::String TabComponent2::getNoteNameFromFrequencyWithTolerance(float frequency)
{
    static const std::array<juce::String, 12> noteNames = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
    
    const float noteFrequencyTolerance = 3.0f;  // Increased tolerance for better note matching

    if (frequency <= 0)
        return "Unknown";

    // Convert frequency to a MIDI note number
    int noteNumber = static_cast<int>(std::round(12.0f * std::log2(frequency / 440.0f))) + 69;
    int octave = noteNumber / 12 - 1;
    juce::String noteName = noteNames[noteNumber % 12];

    // Calculate the frequency of the nearest note
    float closestFrequency = 440.0f * std::pow(2.0f, (noteNumber - 69) / 12.0f);

    // Only return the note name if the detected frequency is within the tolerance
    if (std::abs(frequency - closestFrequency) <= noteFrequencyTolerance)
    {
        return noteName + juce::String(octave);  // e.g., "A4"
    }

    return "Unknown";  // Return "Unknown" if the frequency is outside the tolerance
}

void TabComponent2::updateNoteUI(const juce::String& detectedNote)
{
    if (detectedNote != currentNote)
    {
        currentNote = detectedNote;

        DBG("Updating UI with note: " + detectedNote);

        juce::MessageManager::callAsync([this, detectedNote]()
        {
            noteLabel.setFont(juce::FontOptions(40.0f, juce::Font::bold));
            noteLabel.setColour(juce::Label::textColourId, juce::Colours::cyan);
            noteLabel.setText("Detected Note: " + detectedNote, juce::dontSendNotification);
        });
    }
}

void TabComponent2::timerCallback()
{
    repaint();
}

void TabComponent2::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::darkgrey);

    const std::vector<float>& waveform = yinProcessor.getWaveformBuffer();
    if (!waveform.empty())
    {
        g.setColour(juce::Colours::cyan);
        juce::Path waveformPath;
        const int width = getWidth();
        const int height = getHeight();
        const float centerY = height * 0.5f;

        waveformPath.startNewSubPath(0, centerY);

        for (int i = 0; i < width && i < waveform.size(); ++i)
        {
            const float x = (float)i;
            const float y = centerY - waveform[i] * centerY;
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
