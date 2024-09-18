#include "TabComponent2.hpp"

//==============================================================================
TabComponent2::TabComponent2()
{
    // Set up UI components
    addAndMakeVisible(noteLabel);
    noteLabel.setText("No note detected", juce::dontSendNotification);
    noteLabel.setFont(juce::FontOptions(24.0f, juce::Font::bold));
    noteLabel.setJustificationType(juce::Justification::centred);

    addAndMakeVisible(requiredNoteLabel);
    requiredNoteLabel.setText("Select a scale...", juce::dontSendNotification);
    requiredNoteLabel.setFont(juce::FontOptions(18.0f, juce::Font::italic));
    requiredNoteLabel.setJustificationType(juce::Justification::centred);

    addAndMakeVisible(scaleComboBox);
    scaleComboBox.addItem("C Major", 1);
    scaleComboBox.addItem("A Minor", 2);
    scaleComboBox.addItem("C Major Pentatonic", 3);
    scaleComboBox.addItem("A Minor Pentatonic", 4);
    scaleComboBox.addItem("E Blues", 5);
    scaleComboBox.setJustificationType(juce::Justification::centred);
    scaleComboBox.onChange = [this]() { loadScale(); };  // Load the selected scale

    addAndMakeVisible(resetButton);
    resetButton.setButtonText("Start Again");
    resetButton.onClick = [this]() { resetChallenge(); };

    // Initialize variables
    lastFrequency = 0.0f;
    currentNoteIndex = 0;
    isCorrectNote = false;
    stabilityCounter = 0;
    requiredStabilityCount = 15;  // Number of frames required for stability

    startTimerHz(15);  // Reduced timer for UI updates
}

TabComponent2::~TabComponent2()
{
    stopTimer();
}

void TabComponent2::resized()
{
    auto area = getLocalBounds().reduced(10);

    const int labelHeight = 40;
    const int comboBoxHeight = 30;
    const int buttonHeight = 40;
    const int spacing = 20;

    int totalHeight = labelHeight * 2 + comboBoxHeight + buttonHeight + spacing * 3;
    area.removeFromTop((getHeight() - totalHeight) / 2);  // Center vertically

    placeComponent(noteLabel, area, labelHeight, spacing);
    placeComponent(requiredNoteLabel, area, labelHeight, spacing);
    placeComponent(scaleComboBox, area, comboBoxHeight, spacing);
    placeComponent(resetButton, area, buttonHeight, spacing);
}

void TabComponent2::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::darkgrey);  // Background color
}

void TabComponent2::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    yinProcessor.initialize(sampleRate, samplesPerBlockExpected);
}

void TabComponent2::processAudioBuffer(const juce::AudioSourceChannelInfo& bufferToFill)
{
    if (bufferToFill.buffer != nullptr && bufferToFill.buffer->getNumChannels() > 0)
    {
        for (int sample = 0; sample < bufferToFill.buffer->getNumSamples(); ++sample)
        {
            float sum = 0.0f;
            for (int channel = 0; channel < bufferToFill.buffer->getNumChannels(); ++channel)
                sum += bufferToFill.buffer->getReadPointer(channel)[sample];

            float monoSample = sum / bufferToFill.buffer->getNumChannels();
            float detectedPitch = yinProcessor.processAudioBuffer(&monoSample, 1);

            if (detectedPitch > 0.0f)
            {
                // Use callAsync to ensure UI updates happen on the main thread
                juce::MessageManager::callAsync([this, detectedPitch]()
                {
                    checkNoteInScale(detectedPitch);
                });
            }
        }
    }
}

juce::String TabComponent2::getNoteNameFromFrequencyWithTolerance(float frequency)
{
    static const std::array<juce::String, 12> noteNames = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
    const float referenceA4 = 440.0f;
    const float tolerance = 5.0f;

    int midiNote = static_cast<int>(std::round(12.0f * std::log2(frequency / referenceA4))) + 69;
    int octave = midiNote / 12 - 1;
    juce::String noteName = noteNames[midiNote % 12];

    float closestNoteFrequency = referenceA4 * std::pow(2.0f, (midiNote - 69) / 12.0f);

    if (std::abs(frequency - closestNoteFrequency) <= tolerance)
    {
        return noteName + juce::String(octave);
    }
    else
    {
        return "Unknown";
    }
}

void TabComponent2::checkNoteInScale(float frequency)
{
    juce::String detectedNote = getNoteNameFromFrequencyWithTolerance(frequency);

    // Update the note being played on the UI
    updateNoteUI("Detected: " + detectedNote);

    // Ensure current note index is within bounds
    if (currentNoteIndex >= currentScaleNotes.size()) return;

    // Check if the detected note matches the required note
    if (detectedNote == currentRequiredNote)
    {
        // Increment the stability counter if the note matches
        stabilityCounter++;
    }
    else
    {
        // Reset the stability counter if the note is not stable
        stabilityCounter = 0;
    }

    // Once the note has been stable for enough frames, confirm the note
    if (stabilityCounter >= requiredStabilityCount)
    {
        isCorrectNote = true;
        stabilityCounter = 0;  // Reset the counter for the next note

        // Show "Correct!" and highlight the note in green for 1 second
        juce::MessageManager::callAsync([this]()
        {
            updateNoteUI("Correct!");

            // Move to the next note after 1 second
            juce::Timer::callAfterDelay(1000, [this]()
            {
                isCorrectNote = false;
                currentNoteIndex++;

                if (currentNoteIndex < currentScaleNotes.size())
                {
                    // Update to the next required note
                    currentRequiredNote = currentScaleNotes[currentNoteIndex];
                    updateRequiredNote();  // Update the UI to show the next note
                }
                else
                {
                    // Scale completed
                    updateNoteUI("Scale completed!");
                }
            });
        });
    }
}

void TabComponent2::updateNoteUI(const juce::String& message)
{
    if (noteLabel.getText() != message)
    {
        juce::MessageManager::callAsync([this, message]() {
            noteLabel.setText(message, juce::dontSendNotification);
        });
    }
}

void TabComponent2::resetChallenge()
{
    currentNoteIndex = 0;
    updateNoteUI("Select a scale...");
    resetLabelsToDefault();
    loadScale();
}

void TabComponent2::resetLabelsToDefault()
{
    juce::MessageManager::callAsync([this]() {
        // Remove labels for all scale notes (since we are no longer displaying them)
        noteLabel.setText("No note detected", juce::dontSendNotification);
        requiredNoteLabel.setText("Select a scale...", juce::dontSendNotification);
    });
}

void TabComponent2::loadScale()
{
    stringAndFret.clear();
    currentScaleNotes.clear();
    resetLabelsToDefault();
    currentNoteIndex = 0;

    switch (scaleComboBox.getSelectedId())
    {
        case 1: // C Major Scale
            currentScaleNotes = { "C3", "D3", "E3", "F3", "G3", "A3", "B3" };
            stringAndFret = {
                {"5th string", "3rd fret"}, // C3
                {"4th string", "0 fret"},   // D3
                {"4th string", "2nd fret"}, // E3
                {"4th string", "3rd fret"}, // F3
                {"3rd string", "0 fret"},   // G3
                {"3rd string", "2nd fret"}, // A3
                {"2nd string", "0 fret"}    // B3
            };
            break;
        case 2: // A Minor Scale
            currentScaleNotes = { "A2", "B2", "C3", "D3", "E3", "F3", "G3" };
            stringAndFret = {
                {"5th string", "0 fret"},   // A2
                {"5th string", "2nd fret"}, // B2
                {"5th string", "3rd fret"}, // C3
                {"4th string", "0 fret"},   // D3
                {"4th string", "2nd fret"}, // E3
                {"4th string", "3rd fret"}, // F3
                {"3rd string", "0 fret"}    // G3
            };
            break;
        case 3: // C Major Pentatonic Scale
            currentScaleNotes = { "C3", "D3", "E3", "G3", "A3" };
            stringAndFret = {
                {"5th string", "3rd fret"}, // C3
                {"4th string", "0 fret"},   // D3
                {"4th string", "2nd fret"}, // E3
                {"3rd string", "0 fret"},   // G3
                {"3rd string", "2nd fret"}  // A3
            };
            break;
        case 4: // A Minor Pentatonic Scale
            currentScaleNotes = { "A2", "C3", "D3", "E3", "G3" };
            stringAndFret = {
                {"5th string", "0 fret"},   // A2
                {"5th string", "3rd fret"}, // C3
                {"4th string", "0 fret"},   // D3
                {"4th string", "2nd fret"}, // E3
                {"3rd string", "0 fret"}    // G3
            };
            break;
        case 5: // E Minor Pentatonic Scale
            currentScaleNotes = { "E2", "G2", "A2", "A#2", "B2", "D3" };
            stringAndFret = {
                {"6th string", "0 fret"},   // E2
                {"6th string", "3rd fret"}, // G2
                {"5th string", "0 fret"},   // A2
                {"5th string", "1st fret"}, // A#2
                {"5th string", "2nd fret"}, // B2
                {"4th string", "0 fret"}    // D3
            };
            break;
        default:
            currentScaleNotes.clear();
            stringAndFret.clear();
            break;
    }

    // Only display the first note of the scale to be played
    // After loading the scale, set the first note to be played
        if (!currentScaleNotes.empty())
        {
            currentRequiredNote = currentScaleNotes[currentNoteIndex];
            updateRequiredNote();  // Update the UI to show the first note to be played
        }
}

void TabComponent2::updateRequiredNote()
{
    if (currentNoteIndex < currentScaleNotes.size())
    {
        requiredNoteLabel.setText(
            "Play: " + currentRequiredNote + " on " + stringAndFret[currentNoteIndex].first + " at " + stringAndFret[currentNoteIndex].second,
            juce::dontSendNotification
        );
    }
}

void TabComponent2::placeComponent(juce::Component& comp, juce::Rectangle<int>& area, int height, int spacing)
{
    comp.setBounds(area.removeFromTop(height));
    area.removeFromTop(spacing);
}

void TabComponent2::timerCallback()
{
    // Conditionally repaint only when necessary
    if (isCorrectNote)
    {
        repaint();
    }
}
