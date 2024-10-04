#include "TabComponent2.hpp"

//==============================================================================
TabComponent2::TabComponent2()
{
    //set up UI components
    addAndMakeVisible(noteLabel);
    noteLabel.setText("No note detected", juce::dontSendNotification);
    noteLabel.setFont(juce::FontOptions(24.0f, juce::Font::bold));
    noteLabel.setJustificationType(juce::Justification::centred);
    noteLabel.setColour(juce::Label::textColourId, juce::Colours::black);

    addAndMakeVisible(requiredNoteLabel);
    requiredNoteLabel.setText("Select a scale...", juce::dontSendNotification);
    requiredNoteLabel.setFont(juce::FontOptions(18.0f, juce::Font::italic));
    requiredNoteLabel.setJustificationType(juce::Justification::centred);
    requiredNoteLabel.setColour(juce::Label::textColourId, juce::Colours::black);
    
    addAndMakeVisible(statusLabel);
    statusLabel.setText("", juce::dontSendNotification);
    statusLabel.setFont(juce::FontOptions(24.0f, juce::Font::bold));
    statusLabel.setJustificationType(juce::Justification::centred);
    statusLabel.setColour(juce::Label::textColourId, juce::Colours::black);
    
    addAndMakeVisible(infoButton);
    infoButton.setButtonText("Info");
    infoButton.onClick = [this]() { toggleInfoOverlay(); };
    addAndMakeVisible(infoOverlay);
    infoOverlay.setVisible(false);
    
    addAndMakeVisible(scaleComboBox);
    scaleComboBox.addItem("C Major", 1);
    scaleComboBox.addItem("A Minor", 2);
    scaleComboBox.addItem("C Major Pentatonic", 3);
    scaleComboBox.addItem("A Minor Pentatonic", 4);
    scaleComboBox.addItem("E Blues", 5);
    scaleComboBox.setJustificationType(juce::Justification::centred);
    scaleComboBox.onChange = [this]() { loadScale(); };  //load the selected scale

    addAndMakeVisible(resetButton);
    resetButton.setButtonText("Start Again");
    resetButton.onClick = [this]() { resetChallenge(); };

    //Variables
    lastFrequency = 0.0f;
    currentNoteIndex = 0;
    isCorrectNote = false;
    stabilityCounter = 0;
    requiredStabilityCount = 15;  //number of frames for stability
}

TabComponent2::~TabComponent2()
{
}

//resizes UI components
void TabComponent2::resized()
{

    auto area = getLocalBounds().reduced(20);

    juce::FlexBox flexBox;
    flexBox.flexDirection = juce::FlexBox::Direction::column;
    flexBox.justifyContent = juce::FlexBox::JustifyContent::center;
    flexBox.alignItems = juce::FlexBox::AlignItems::center;
    
    flexBox.items.add(juce::FlexItem(noteLabel).withMinWidth(300).withMinHeight(40).withMargin(juce::FlexItem::Margin(10)));
    flexBox.items.add(juce::FlexItem(statusLabel).withMinWidth(300).withMinHeight(40).withMargin(juce::FlexItem::Margin(10)));
    flexBox.items.add(juce::FlexItem(requiredNoteLabel).withMinWidth(300).withMinHeight(40).withMargin(juce::FlexItem::Margin(10)));
    flexBox.items.add(juce::FlexItem(scaleComboBox).withMinWidth(200).withMinHeight(30).withMargin(juce::FlexItem::Margin(10)));
    flexBox.items.add(juce::FlexItem(resetButton).withMinWidth(150).withMinHeight(40).withMargin(juce::FlexItem::Margin(10)));
    flexBox.items.add(juce::FlexItem(infoButton).withMinWidth(150).withMinHeight(40).withMargin(juce::FlexItem::Margin(10)));

    flexBox.performLayout(area);

    infoOverlay.setBounds(getLocalBounds());
}


//background colour
void TabComponent2::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour::fromRGB(240, 230, 200));
}

//starts yin processing
void TabComponent2::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    yinProcessor.initialize(sampleRate, samplesPerBlockExpected);
}

//audio processing for note detection from YINAudioComponent
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

// Takes the detected pitch and matches it to the correct note
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

// Checks if the detected pitch corresponds to the correct note
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

// Move to the next note in the scale challenge
void TabComponent2::moveToNextNote()
{
    if (currentNoteIndex < currentScaleNotes.size() - 1)
    {
        currentNoteIndex++;
        currentRequiredNote = currentScaleNotes[currentNoteIndex];
        isCorrectNote = false;

        showMessageWithDelay("Correct!", 1000, [this]()
        {
            updateRequiredNote();
            showMessageWithDelay("", 2000, [this]()
            {
                isCorrectNote = true;
            });
        });
    }
    else if (!scaleCompleted)
        {
        
            scaleCompleted = true;
            showMessageWithDelay("Scale completed!", 4000, [this]()
            {
                updateStatusUI("");
            });
        }
}

//UI update for detected note
void TabComponent2::updateNoteUI(const juce::String& message)
{
    if (noteLabel.getText() != message)
    {
        juce::MessageManager::callAsync([this, message]()
        {
            noteLabel.setText(message, juce::dontSendNotification);
            repaint();
        });
    }
}

//UI update for status
void TabComponent2::updateStatusUI(const juce::String& message)
{
    if (statusLabel.getText() != message)
    {
        juce::MessageManager::callAsync([this, message]()
        {
            statusLabel.setText(message, juce::dontSendNotification);
            repaint();
        });
    }
}

//Reset the scale challenge
void TabComponent2::resetChallenge()
{
    currentNoteIndex = 0;
    isCorrectNote = false;
    scaleCompleted = false;

    if (scaleComboBox.getSelectedId() > 0)
    {
        loadScale();
        updateRequiredNote();
    }
}

//loads the selected scale
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
                {"4th string", "open"},     // D3
                {"4th string", "2nd fret"}, // E3
                {"4th string", "3rd fret"}, // F3
                {"3rd string", "open"},     // G3
                {"3rd string", "2nd fret"}, // A3
                {"2nd string", "open"}      // B3
            };
            break;
            
        case 2: // A Minor Scale
            currentScaleNotes = { "A2", "B2", "C3", "D3", "E3", "F3", "G3" };
            stringAndFret = {
                {"5th string", "open"},     // A2
                {"5th string", "2nd fret"}, // B2
                {"5th string", "3rd fret"}, // C3
                {"4th string", "open"},     // D3
                {"4th string", "2nd fret"}, // E3
                {"4th string", "3rd fret"}, // F3
                {"3rd string", "open"}      // G3
            };
            break;

        case 3: // C Major Pentatonic Scale
            currentScaleNotes = { "C3", "D3", "E3", "G3", "A3" };
            stringAndFret = {
                {"5th string", "3rd fret"}, // C3
                {"4th string", "open"},     // D3
                {"4th string", "2nd fret"}, // E3
                {"3rd string", "open"},     // G3
                {"3rd string", "2nd fret"}  // A3
            };
            break;

        case 4: // A Minor Pentatonic Scale
            currentScaleNotes = { "A2", "C3", "D3", "E3", "G3" };
            stringAndFret = {
                {"5th string", "open"},     // A2
                {"5th string", "3rd fret"}, // C3
                {"4th string", "open"},     // D3
                {"4th string", "2nd fret"}, // E3
                {"3rd string", "open"}      // G3
            };
            break;

        case 5: // E Blues Scale
            currentScaleNotes = { "E2", "G2", "A2", "A#2", "B2", "D3" };
            stringAndFret = {
                {"6th string", "open"},     // E2
                {"6th string", "3rd fret"}, // G2
                {"5th string", "open"},     // A2
                {"5th string", "1st fret"}, // A#2
                {"5th string", "2nd fret"}, // B2
                {"4th string", "open"}      // D3
            };
            break;

        default:
            currentScaleNotes.clear();
            stringAndFret.clear();
            break;
    }

    if (!currentScaleNotes.empty())
    {
        currentRequiredNote = currentScaleNotes[currentNoteIndex];
        updateRequiredNote();
    }
}

// Updates the required note on the UI
void TabComponent2::updateRequiredNote()
{
    if (currentNoteIndex < currentScaleNotes.size())
    {
        requiredNoteLabel.setText(
            "Play: " + currentRequiredNote + " on " + stringAndFret[currentNoteIndex].first + " at " + stringAndFret[currentNoteIndex].second,
            juce::dontSendNotification
        );
        repaint();  // Repaint when the required note changes
    }
}

// Info overlay
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

//show a message with a delay
void TabComponent2::showMessageWithDelay(const juce::String& message, int delay, std::function<void()> callback)
{
    updateStatusUI(message);
    juce::Timer::callAfterDelay(delay, [callback]() { callback(); });
}

void TabComponent2::releaseResources()
{

}
