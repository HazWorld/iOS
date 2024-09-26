#include "TabComponent2.hpp"


//==============================================================================
TabComponent2::TabComponent2()
{
    // Set up UI components
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
    // Add padding around the edges of the component
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

    // Perform the layout inside the area
    flexBox.performLayout(area);

    // Ensure the info overlay covers the full component
    infoOverlay.setBounds(getLocalBounds());
}

void TabComponent2::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour::fromRGB(240, 230, 200));
}


void TabComponent2::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    yinProcessor.initialize(sampleRate, samplesPerBlockExpected);
}


//Audio processing for note detection from YINAudioComponent
void TabComponent2::processAudioBuffer(const juce::AudioSourceChannelInfo& bufferToFill)
{
    //checks buffers
    if (bufferToFill.buffer != nullptr && bufferToFill.buffer->getNumChannels() > 0)
    {
        //audio smaple loop
        for (int sample = 0; sample < bufferToFill.buffer->getNumSamples(); ++sample)
        {
            float sum = 0.0f;
            //averages signal for mono processing
            for (int channel = 0; channel < bufferToFill.buffer->getNumChannels(); ++channel)
                sum += bufferToFill.buffer->getReadPointer(channel)[sample];

            float monoSample = sum / bufferToFill.buffer->getNumChannels();
            
            //feeding mono sample into yin processor
            float detectedPitch = yinProcessor.processAudioBuffer(&monoSample, 1);

            //calls checkNoteInScale function for the detected pitch
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
    //reference pitch
    const float referenceA4 = 440.0f;
    //frequency tolerence for detected pitches
    const float tolerance = 5.0f;

    //calculates the semitones above or below reference
    int midiNote = static_cast<int>(std::round(12.0f * std::log2(frequency / referenceA4))) + 69;
    //finds the octave
    int octave = midiNote / 12 - 1;
    //maps the note name to the midi note
    juce::String noteName = noteNames[midiNote % 12];
    //calculates the frequency required for the midi note
    float closestNoteFrequency = referenceA4 * std::pow(2.0f, (midiNote - 69) / 12.0f);

    //checks detected frequency against the note frequency and returns note and octave
    if (std::abs(frequency - closestNoteFrequency) <= tolerance)
    {
        return noteName + juce::String(octave);
    }
    else
    {
        return "Unknown";
    }
}

//checks the note corresponds to the current required note for the scale selected
void TabComponent2::checkNoteInScale(float frequency)
{
    //finds the detected note and updates the UI to show it
    juce::String detectedNote = getNoteNameFromFrequencyWithTolerance(frequency);
    updateNoteUI("Detected: " + detectedNote);

    //checks if the scale has finished
    if (currentNoteIndex >= currentScaleNotes.size()) return;

    //checks it note played is correct and moves to next note
    if (detectedNote == currentRequiredNote)
    {
        isCorrectNote = true;
        moveToNextNote();
    }
}

// handles the itteration between notes when moving through the scale challenge
void TabComponent2::moveToNextNote()
{
    //check if there is more notes in the scale
    if (currentNoteIndex < currentScaleNotes.size() - 1)
    {
        // moves onto the next note
        currentNoteIndex++;
        currentRequiredNote = currentScaleNotes[currentNoteIndex];
        isCorrectNote = false;  // Resets after correct note is played
        
        //updtes timer after delay
        juce::Timer::callAfterDelay(1000, [this]() {
            updateRequiredNote();
            updateStatusUI("Correct!");
             //resets status
            juce::Timer::callAfterDelay(2000, [this]() {
                updateStatusUI("");
                isCorrectNote = true;  // Set after message clears
            });
        });
    }
    else
    {
        //alerts the user that the scale is complete
        juce::Timer::callAfterDelay(1000, [this]() {
            updateStatusUI("Scale completed!");
            isCorrectNote = false;
        });
    }
}

//handles UI update for notes on JUCE message thread
void TabComponent2::updateNoteUI(const juce::String& message)
{
    if (noteLabel.getText() != message)
    {
        juce::MessageManager::callAsync([this, message]() {
            noteLabel.setText(message, juce::dontSendNotification);
        });
    }
}

//hadnles UI update for status on the message thread
void TabComponent2::updateStatusUI(const juce::String& message)
{
    if (statusLabel.getText() != message)
    {
        juce::MessageManager::callAsync([this, message]() {
            statusLabel.setText(message, juce::dontSendNotification);
        });
    }
}

//resets the current scale challenge
void TabComponent2::resetChallenge()
{
    currentNoteIndex = 0;
    isCorrectNote = false;

    // Reload the current scale
    if (scaleComboBox.getSelectedId() > 0)
    {
        loadScale();
        updateRequiredNote();
    }
}

//handles the loading of scales with relevent information about the scale
//add more scales here //////
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
        case 5: // E Minor Pentatonic Scale
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

    //only displays the first note of the scale to be played
    //after loading sets the first note to be played
        if (!currentScaleNotes.empty())
        {
            currentRequiredNote = currentScaleNotes[currentNoteIndex];
            updateRequiredNote();
        }
}

//updates the note required to be played on the UI
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

//Info overlay for tab 2
void TabComponent2::toggleInfoOverlay()
{
    if (!infoOverlay.isVisible())
    {
        //text for info overlay
        infoOverlay.setInfoContent("This is the information for TabComponent2.\n\nHere you can play scales and detect notes.");
        infoOverlay.setVisible(true);
        infoOverlay.toFront(true);
    }
    else
    {
        infoOverlay.setVisible(false);
    }
}


void TabComponent2::timerCallback()
{
    //repaint only when necessary
    if (isCorrectNote)
    {
        repaint();
    }
}

void TabComponent2::releaseResources()
{
    stopTimer();
}

