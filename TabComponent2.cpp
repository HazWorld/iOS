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
    
    addAndMakeVisible(statusLabel);
    statusLabel.setText("", juce::dontSendNotification);
    statusLabel.setFont(juce::FontOptions(24.0f, juce::Font::bold));
    statusLabel.setJustificationType(juce::Justification::centred);
    
    addAndMakeVisible(infoButton);
    infoButton.setButtonText("Info");
    infoButton.onClick = [this]() { toggleInfoOverlay(); };
    addAndMakeVisible(infoOverlay);
    infoOverlay.setVisible(false);  // Initially hidden
    

    addAndMakeVisible(scaleComboBox);
    scaleComboBox.addItem("C Major", 1);
    scaleComboBox.addItem("A Minor", 2);
    scaleComboBox.addItem("C Major Pentatonic", 3);
    scaleComboBox.addItem("A Minor Pentatonic", 4);
    scaleComboBox.addItem("E Blues", 5);
    //add more scales here
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
    
    // Calculate the total height of components
    const int labelHeight = 40;
    const int comboBoxHeight = 30;
    const int buttonHeight = 40;
    const int spacing = 20;
    
    int totalHeight = labelHeight * 2 + comboBoxHeight + buttonHeight + spacing * 3;
    
    // Vertically center the content
    area.removeFromTop((getHeight() - totalHeight) / 2);
    
    // Now place the components in the vertically centered area
    placeComponent(noteLabel, area, labelHeight, spacing);
    placeComponent(requiredNoteLabel, area, labelHeight, spacing);
    placeComponent(scaleComboBox, area, comboBoxHeight, spacing);
    placeComponent(resetButton, area, buttonHeight, spacing);
    placeComponent(infoButton, area, buttonHeight, spacing);
    
    // Ensure the overlay covers the full bounds
    infoOverlay.setBounds(getLocalBounds());
}

void TabComponent2::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::darkgrey);  // Background color
}


void TabComponent2::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    yinProcessor.initialize(sampleRate, samplesPerBlockExpected);
}


//Audio processing for note detection from YIN
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
                juce::MessageManager::callAsync([this, detectedPitch]()
                {
                    checkNoteInScale(detectedPitch);
                });
            }
        }
    }
}


//takes the detected pitch and matches it to the correct note
// returns the note plaus the octive of that note
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
    updateNoteUI("Detected: " + detectedNote);

    if (currentNoteIndex >= currentScaleNotes.size()) return;

    if (detectedNote == currentRequiredNote)
    {
        isCorrectNote = true;
        moveToNextNote();
        
    }
}

void TabComponent2::showMessageWithDelay(const juce::String& message, int delay, std::function<void()> callback)
{
    updateNoteUI(message);
    juce::Timer::callAfterDelay(delay, callback);
}

void TabComponent2::moveToNextNote()
{
    if (currentNoteIndex < currentScaleNotes.size() - 1)
    {
        currentNoteIndex++;
        currentRequiredNote = currentScaleNotes[currentNoteIndex];
        juce::Timer::callAfterDelay(1000, [this]() {
            updateRequiredNote();
            updateStatusUI("Correct!");  // Show "Correct!" in status label
            
            juce::Timer::callAfterDelay(2000, [this]() {
                updateStatusUI("");
            });
        });
    }
    else
    {
        juce::Timer::callAfterDelay(1000, [this]() {
            updateStatusUI("Scale completed!");
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

void TabComponent2::updateStatusUI(const juce::String& message)
{
    if (statusLabel.getText() != message)
    {
        juce::MessageManager::callAsync([this, message]() {
            statusLabel.setText(message, juce::dontSendNotification);
        });
    }
}

void TabComponent2::resetChallenge()
{
    currentNoteIndex = 0;  // Restart from the first note
    updateNoteUI("Starting scale again...");
    resetLabelsToDefault();  // Reset the labels without clearing the scale

    // Reload the current scale without resetting the ComboBox
    if (scaleComboBox.getSelectedId() > 0)
    {
        loadScale();  // Keep the currently selected scale and restart
    }
}

void TabComponent2::resetLabelsToDefault()
{
    juce::MessageManager::callAsync([this]() {
        noteLabel.setText("No note detected", juce::dontSendNotification);
        requiredNoteLabel.setText("Select a scale...", juce::dontSendNotification);
        statusLabel.setText("", juce::dontSendNotification);
    });
}

void TabComponent2::loadScale()
{
    stringAndFret.clear();
    currentScaleNotes.clear();
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

void TabComponent2::toggleInfoOverlay()
{
    if (!infoOverlay.isVisible())
    {
        infoOverlay.setInfoContent("This is the information for TabComponent2.\n\nHere you can play scales and detect notes.");
        infoOverlay.setVisible(true);
        infoOverlay.toFront(true);
    }
    else
    {
        infoOverlay.setVisible(false);
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
