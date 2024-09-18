#include "TabComponent3.hpp"

//==============================================================================
// Constructor
TabComponent3::TabComponent3()
{
    // Set up label for displaying tempo information
    addAndMakeVisible(label);
    label.setText("Tap to set tempo or start playing", juce::dontSendNotification);
    label.setJustificationType(juce::Justification::centred);

    // Set up tap button for tap-tempo detection
    addAndMakeVisible(tapButton);
    tapButton.setButtonText("Tap Tempo");
    tapButton.onClick = [this]() { tapTempo(); };

    // Set up slider for manual tempo input
    addAndMakeVisible(tempoSlider);
    tempoSlider.setRange(40.0, 240.0, 1.0); // Tempo range from 40 BPM to 240 BPM
    tempoSlider.setValue(120.0);            // Default tempo is 120 BPM
    tempoSlider.onValueChange = [this]() { setManualTempo(); };
    tempoSlider.setTextValueSuffix(" BPM");

    // Start the timer to update the visual cue and check tempo deviation
    startTimerHz(30);  // Check every ~33ms (30 FPS)
}

//==============================================================================
// Component Resized
void TabComponent3::resized()
{
    // Arrange UI components
    auto area = getLocalBounds();
    label.setBounds(area.removeFromTop(50));
    tapButton.setBounds(area.removeFromTop(50).reduced(10));
    tempoSlider.setBounds(area.removeFromTop(50).reduced(10));

    // Visual cue area
    visualCueArea = area.reduced(10);  // The remaining area will be used for visual cue
}

//==============================================================================
// Paint Component (updates background color and visual cue)
void TabComponent3::paint(juce::Graphics& g)
{
    // Set background color based on tempo deviation and whether the player is playing
    if (isPlaying && isDeviatingFromTempo)
        g.fillAll(juce::Colours::red);   // Turn red if deviating from the set tempo
    else
        g.fillAll(juce::Colours::lightgrey); // Default light grey when playing correctly or not playing

    // Draw visual cue for the current tempo
    if (showVisualCue)
    {
        g.setColour(juce::Colours::green);
        g.fillEllipse(visualCueArea.toFloat());  // Simple pulsating circle as a visual cue
    }
}

//==============================================================================
// Tap tempo function to detect tempo based on user input
void TabComponent3::tapTempo()
{
    using namespace std::chrono;
    
    // Capture the current time
    auto now = steady_clock::now();

    // Store the tap time
    tapTimes.push_back(now);

    // Only keep the last 2 tap times
    if (tapTimes.size() > 2)
        tapTimes.erase(tapTimes.begin());

    // Calculate the tempo based on the interval between the taps
    if (tapTimes.size() == 2)
    {
        auto duration = duration_cast<milliseconds>(tapTimes[1] - tapTimes[0]).count();
        if (duration > 0)
        {
            detectedTempo = 60000.0 / static_cast<double>(duration);  // Convert duration to BPM
            label.setText("Detected Tempo: " + juce::String(detectedTempo, 2) + " BPM", juce::dontSendNotification);
        }
    }
}

//==============================================================================
// Set manual tempo from slider
void TabComponent3::setManualTempo()
{
    detectedTempo = tempoSlider.getValue();
    label.setText("Manual Tempo: " + juce::String(detectedTempo, 2) + " BPM", juce::dontSendNotification);
}

//==============================================================================
// Detect peaks in magnitude and calculate the tempo from peaks
void TabComponent3::detectTempoFromPeaks(float magnitude)
{
    // Check if the magnitude exceeds the threshold, meaning the player is playing
    isPlaying = (magnitude > magnitudeThreshold);

    if (isPlaying)
    {
        // Detect peaks (when the magnitude crosses the previous magnitude)
        if (magnitude > previousMagnitude && previousMagnitude < magnitudeThreshold)
        {
            using namespace std::chrono;
            auto now = steady_clock::now();

            // Store the peak time and calculate tempo from successive peaks
            tapTimes.push_back(now);
            if (tapTimes.size() > 2)
                tapTimes.erase(tapTimes.begin());

            if (tapTimes.size() == 2)
            {
                auto duration = duration_cast<milliseconds>(tapTimes[1] - tapTimes[0]).count();
                if (duration > 0)
                {
                    currentTempo = 60000.0 / static_cast<double>(duration);  // Convert duration to BPM
                }
            }
        }
    }
    
    previousMagnitude = magnitude;
}

//==============================================================================
// Simulate tempo deviation detection based on actual playing tempo
bool TabComponent3::isPlayerInTempo()
{
    // Allow a 5% deviation tolerance from the detected tempo
    double tolerance = detectedTempo * 0.05;
    return (currentTempo >= detectedTempo - tolerance && currentTempo <= detectedTempo + tolerance);
}

//==============================================================================
// Timer callback to update the visual cue and check tempo deviation
void TabComponent3::timerCallback()
{
    if (isPlaying)
    {
        if (!isPlayerInTempo())
        {
            isDeviatingFromTempo = true;
        }
        else
        {
            isDeviatingFromTempo = false;
        }

        // Show visual cue when the player is playing and in tempo
        showVisualCue = !isDeviatingFromTempo;
    }
    else
    {
        isDeviatingFromTempo = false;
        showVisualCue = false;  // Hide visual cue if the player is not playing
    }

    // Repaint the screen to update the background color and visual cue
    repaint();
}

//==============================================================================
// Audio processing to detect magnitude and feed it into the peak detection
void TabComponent3::processAudioBuffer(const juce::AudioSourceChannelInfo& bufferToFill)
{
    if (bufferToFill.buffer != nullptr && bufferToFill.buffer->getNumChannels() > 0)
    {
        float magnitude = 0.0f;
        
        // Calculate the magnitude (RMS) of the audio signal
        for (int channel = 0; channel < bufferToFill.buffer->getNumChannels(); ++channel)
        {
            auto* samples = bufferToFill.buffer->getReadPointer(channel);
            for (int sample = 0; sample < bufferToFill.buffer->getNumSamples(); ++sample)
            {
                magnitude += std::abs(samples[sample]);
            }
        }
        magnitude /= (bufferToFill.buffer->getNumChannels() * bufferToFill.buffer->getNumSamples());

        // Detect tempo from peaks in the magnitude
        detectTempoFromPeaks(magnitude);
    }
}

void TabComponent3::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    this->sampleRate = static_cast<int>(sampleRate);
}
