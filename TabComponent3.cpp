#include "TabComponent3.hpp"

//==============================================================================
// Define configurable parameters at the top for easy changes
const float minMagnitudeThreshold = 0.07f;     // Minimum input magnitude for signal detection
const float debounceDelayMs = 300;             // Minimum delay between valid peaks (debounce)
const int maxTapTimesSize = 6;                 // Maximum number of peaks to store in the buffer
const float initialSmoothingFactor = 0.1f;     // Smoothing factor for magnitude and tempo
const float aggressiveSmoothingFactor = 0.3f;  // Higher smoothing factor for large tempo shifts
const float dynamicThresholdDecay = 0.98f;     // Decay rate for dynamic threshold
const float thresholdScaling = 0.7f;           // Scaling for dynamic threshold adjustment

//==============================================================================
// Constructor
TabComponent3::TabComponent3()
{
    addAndMakeVisible(label);
    label.setText("Adjust Tempo and Start Playing", juce::dontSendNotification);
    label.setJustificationType(juce::Justification::centred);

    addAndMakeVisible(tempoSlider);
    tempoSlider.setRange(40.0, 240.0, 1.0);
    tempoSlider.setValue(120.0);
    tempoSlider.onValueChange = [this]() { setManualTempo(); };
    tempoSlider.setTextValueSuffix(" BPM");

    startTimerHz(30);  // 30 FPS for visual updates
}

//==============================================================================
// Resized: Use FlexBox to layout components
void TabComponent3::resized()
{
    auto area = getLocalBounds().reduced(20);

    juce::FlexBox flexBox;
    flexBox.flexDirection = juce::FlexBox::Direction::column;
    flexBox.justifyContent = juce::FlexBox::JustifyContent::center;
    flexBox.alignItems = juce::FlexBox::AlignItems::center;

    flexBox.items.add(juce::FlexItem(label).withMinWidth(300).withMinHeight(40).withMargin(juce::FlexItem::Margin(10)));
    flexBox.items.add(juce::FlexItem(tempoSlider).withMinWidth(300).withMinHeight(40).withMargin(juce::FlexItem::Margin(10)));

    flexBox.performLayout(area);

    visualCueArea = area.removeFromBottom(200).reduced(10);
}

//==============================================================================
// Paint: Handle UI visuals and tempo pulse
void TabComponent3::paint(juce::Graphics& g)
{
    g.fillAll(isPlaying && isDeviatingFromTempo ? juce::Colours::red : juce::Colour::fromRGB(240, 230, 200));

    if (showVisualCue)
    {
        auto pulseAlpha = 0.5f + 0.5f * static_cast<float>(std::sin(juce::Time::getMillisecondCounterHiRes() * 0.001 * currentTempo));
        g.setColour(juce::Colours::green.withAlpha(pulseAlpha));
        g.fillEllipse(visualCueArea.toFloat());
    }
}

//==============================================================================
// Manual Tempo Input
void TabComponent3::setManualTempo()
{
    detectedTempo = tempoSlider.getValue();
    juce::MessageManager::callAsync([this]() {
        label.setText("Manual Tempo: " + juce::String(detectedTempo, 2) + " BPM", juce::dontSendNotification);
        repaint();
    });
}

//==============================================================================
// Peak Detection and Tempo Calculation
void TabComponent3::detectTempoFromPeaks(float magnitude)
{
    adjustThreshold(magnitude);
    auto now = std::chrono::steady_clock::now();

    if (magnitude > dynamicThreshold && magnitude > previousMagnitude &&
        std::chrono::duration_cast<std::chrono::milliseconds>(now - lastPeakTime).count() > debounceDelayMs)
    {
        lastPeakTime = now;

        tapTimes.push_back(now);
        if (tapTimes.size() > maxTapTimesSize)
            tapTimes.erase(tapTimes.begin());

        if (tapTimes.size() >= 2)
        {
            double totalDuration = 0.0;
            for (size_t i = 1; i < tapTimes.size(); ++i)
            {
                totalDuration += std::chrono::duration_cast<std::chrono::milliseconds>(tapTimes[i] - tapTimes[i - 1]).count();
            }
            double averageDuration = totalDuration / (tapTimes.size() - 1);
            float newTempo = 60000.0 / averageDuration;

            float smoothingFactor = std::abs(currentTempo - newTempo) > 20.0f ? aggressiveSmoothingFactor : initialSmoothingFactor;
            currentTempo = smoothingFactor * newTempo + (1.0f - smoothingFactor) * currentTempo;

            juce::MessageManager::callAsync([this]() {
                label.setText("Detected Tempo: " + juce::String(currentTempo, 2) + " BPM", juce::dontSendNotification);
                repaint();
            });
        }
    }

    previousMagnitude = magnitude;
}

//==============================================================================
// Adjust Dynamic Threshold
void TabComponent3::adjustThreshold(float magnitude)
{
    static float runningAverage = 0.0f;
    runningAverage = initialSmoothingFactor * magnitude + (1.0f - initialSmoothingFactor) * runningAverage;
    dynamicThreshold = std::max(dynamicThreshold * dynamicThresholdDecay, runningAverage * thresholdScaling);
}

//==============================================================================
// Check if player is in tempo
bool TabComponent3::isPlayerInTempo()
{
    double tolerance = detectedTempo * 0.05;  // 5% tolerance of the detected tempo
    return (std::abs(currentTempo - detectedTempo) <= tolerance);
}

//==============================================================================
// Timer Callback for UI updates
void TabComponent3::timerCallback()
{
    isDeviatingFromTempo = isPlaying && !isPlayerInTempo();
    showVisualCue = isPlaying && isPlayerInTempo();  // Show green only if playing and in tempo

    juce::MessageManager::callAsync([this]() { repaint(); });
}

//==============================================================================
// Audio Processing
void TabComponent3::processAudioBuffer(const juce::AudioSourceChannelInfo& bufferToFill)
{
    if (bufferToFill.buffer != nullptr && bufferToFill.buffer->getNumChannels() > 0)
    {
        float magnitude = 0.0f;
        for (int channel = 0; channel < bufferToFill.buffer->getNumChannels(); ++channel)
        {
            auto* samples = bufferToFill.buffer->getReadPointer(channel);
            for (int sample = 0; sample < bufferToFill.buffer->getNumSamples(); ++sample)
            {
                magnitude += std::abs(samples[sample]);
            }
        }
        magnitude /= (bufferToFill.buffer->getNumChannels() * bufferToFill.buffer->getNumSamples());

        // Ignore signals below the minimum threshold
        if (magnitude > minMagnitudeThreshold)
        {
            isPlaying = true;

            // Apply smoothing
            constexpr float smoothingFactor = 0.1f;
            smoothedMagnitude = smoothingFactor * magnitude + (1.0f - smoothingFactor) * smoothedMagnitude;

            detectTempoFromPeaks(smoothedMagnitude);  // Detect tempo only if magnitude exceeds threshold
        }
        else
        {
            isPlaying = false;  // No sound detected
        }
    }
    else
    {
        isPlaying = false;
    }
}

void TabComponent3::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    this->sampleRate = static_cast<int>(sampleRate);
}

void TabComponent3::releaseResources()
{
    // Stops the timer to prevent further UI updates
    stopTimer();
}
