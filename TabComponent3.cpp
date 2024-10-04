#include "TabComponent3.hpp"


const float minMagnitudeThreshold = 0.07f;     //minimum input magnitude for signal detection
const float debounceDelayMs = 300;             //minimum delay between valid peaks
const int maxTapTimesSize = 6;                 //maximum number of peaks stored in buffer
const float initialSmoothingFactor = 0.1f;     //smoothing factor
const float aggressiveSmoothingFactor = 0.3f;  //higher smoothing factor for large tempo shifts
const float dynamicThresholdDecay = 0.98f;     //decay rate for dynamic threshold
const float thresholdScaling = 0.7f;           //scaling for dynamic threshold


//constructor
TabComponent3::TabComponent3()
{
    //sets label for detected tempo
    addAndMakeVisible(detectedTempoLabel);
    detectedTempoLabel.setFont(juce::FontOptions(24.0f, juce::Font::bold));
    detectedTempoLabel.setText("Detected Tempo: 0 BPM", juce::dontSendNotification);
    detectedTempoLabel.setJustificationType(juce::Justification::centred);

    //sets label for set tempo
    addAndMakeVisible(setTempoLabel);
    setTempoLabel.setFont(juce::FontOptions(18.0f, juce::Font::plain));
    setTempoLabel.setText("Set Tempo: 120 BPM", juce::dontSendNotification);
    setTempoLabel.setJustificationType(juce::Justification::centred);

    //slider setup
    addAndMakeVisible(tempoSlider);
    tempoSlider.setRange(40.0, 240.0, 1.0);
    tempoSlider.setValue(120.0);
    tempoSlider.onValueChange = [this]() { setManualTempo(); };
    tempoSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);

    //info button
    addAndMakeVisible(infoButton);
    infoButton.setButtonText("Info");
    infoButton.onClick = [this]() { toggleInfoOverlay(); };

    addAndMakeVisible(infoOverlay);
    infoOverlay.setVisible(false);
}

//UI layout
void TabComponent3::resized()
{
    
    auto area = getLocalBounds().reduced(20);

    
    juce::FlexBox flexBox;
    flexBox.flexDirection = juce::FlexBox::Direction::column;
    flexBox.justifyContent = juce::FlexBox::JustifyContent::center;
    flexBox.alignItems = juce::FlexBox::AlignItems::center;

    flexBox.items.add(juce::FlexItem(detectedTempoLabel).withMinWidth(300).withMinHeight(50).withMargin(juce::FlexItem::Margin(10)));
    flexBox.items.add(juce::FlexItem(setTempoLabel).withMinWidth(300).withMinHeight(40).withMargin(juce::FlexItem::Margin(10)));
    flexBox.items.add(juce::FlexItem(tempoSlider).withMinWidth(300).withMinHeight(40).withMargin(juce::FlexItem::Margin(10)));
    flexBox.items.add(juce::FlexItem(infoButton).withMinWidth(150).withMinHeight(40).withMargin(juce::FlexItem::Margin(10)));

    flexBox.performLayout(area);

    infoOverlay.setBounds(getLocalBounds());
}


//UI with reaction to tempo matching
void TabComponent3::paint(juce::Graphics& g)
{
    float deviationFactor = std::abs(currentTempo - detectedTempo) / (detectedTempo * 0.1f);
    deviationFactor = juce::jlimit(0.0f, 1.0f, deviationFactor);

    juce::Colour backgroundColour = juce::Colours::red.interpolatedWith(juce::Colours::green, 1.0f - deviationFactor);
    g.fillAll(backgroundColour);
}


//sets manual tempo from slider
void TabComponent3::setManualTempo()
{
    detectedTempo = tempoSlider.getValue();
    setTempoLabel.setText("Set Tempo: " + juce::String(detectedTempo, 2) + " BPM", juce::dontSendNotification);
    repaint();
}

//info overlay
void TabComponent3::toggleInfoOverlay()
{
    if (!infoOverlay.isVisible())
    {
        infoOverlay.setInfoContent("This is the Tempo Tracker!\n\nHere, you can set a tempo you want to play at, then the closer you are to that tempo, the greener the screen will get!!\n\nSet the tempo with the slider then start playing!"); // change the message here
        infoOverlay.setVisible(true);
        infoOverlay.toFront(true);
    }
    else
    {
        infoOverlay.setVisible(false);
    }
}


//peak detection and tempo calculation
void TabComponent3::detectTempoFromPeaks(float magnitude)
{
    adjustThreshold(magnitude);
    //current time
    auto now = std::chrono::steady_clock::now();

    //peak detection
    //checks for threshold and previous peaks
    if (magnitude > dynamicThreshold && magnitude > previousMagnitude &&
        std::chrono::duration_cast<std::chrono::milliseconds>(now - lastPeakTime).count() > debounceDelayMs)
    {
        lastPeakTime = now;

        //stores the last peak time, erasing old peaks to maintain buffer
        tapTimes.push_back(now);
        if (tapTimes.size() > maxTapTimesSize)
            tapTimes.erase(tapTimes.begin());

        //tempo calculation with at least 2 peaks
        if (tapTimes.size() >= 2)
        {
            double totalDuration = 0.0;
            for (size_t i = 1; i < tapTimes.size(); ++i)
            {
                //time between peaks
                totalDuration += std::chrono::duration_cast<std::chrono::milliseconds>(tapTimes[i] - tapTimes[i - 1]).count();
            }
            
            //averages time between peaks
            double averageDuration = totalDuration / (tapTimes.size() - 1);
            //conversion to bpm
            float newTempo = 60000.0 / averageDuration;

            //smooths detected tempo
            float smoothingFactor = std::abs(currentTempo - newTempo) > 20.0f ? aggressiveSmoothingFactor : initialSmoothingFactor;
            currentTempo = smoothingFactor * newTempo + (1.0f - smoothingFactor) * currentTempo;

            //updates UI with detected tempo
            juce::MessageManager::callAsync([this]() {
                detectedTempoLabel.setText("Detected Tempo: " + juce::String(currentTempo, 2) + " BPM", juce::dontSendNotification);
                repaint();
            });
        }
    }

    previousMagnitude = magnitude;
}

//dynamic threshold implementation
void TabComponent3::adjustThreshold(float magnitude)
{
    //smoothing applied to the threshold sdjustments
    float smoothingFactor = std::abs(currentTempo - detectedTempo) > 20.0f ? aggressiveSmoothingFactor : initialSmoothingFactor;
    //average of magnitudes calculated
    static float runningAverage = 0.0f;
    runningAverage = smoothingFactor * magnitude + (1.0f - smoothingFactor) * runningAverage;
    
    //dynamic threshold calculated
    dynamicThreshold = std::max(dynamicThreshold * dynamicThresholdDecay, runningAverage * thresholdScaling);
}


//audio processing buffer
void TabComponent3::processAudioBuffer(const juce::AudioSourceChannelInfo& bufferToFill)
{
    //buffer check
    if (bufferToFill.buffer != nullptr && bufferToFill.buffer->getNumChannels() > 0)
    {
        float magnitude = 0.0f;
        auto numChannels = bufferToFill.buffer->getNumChannels();
        auto numSamples = bufferToFill.buffer->getNumSamples();

        
        for (int channel = 0; channel < numChannels; ++channel)
        {
            auto* samples = bufferToFill.buffer->getReadPointer(channel);
            for (int sample = 0; sample < numSamples; ++sample)
            {
                magnitude += std::abs(samples[sample]);
            }
        }

        //magnitude calculation with smoothing
        magnitude /= (numChannels * numSamples);

        
        if (magnitude > minMagnitudeThreshold)
        {
            smoothedMagnitude = 0.1f * magnitude + 0.9f * smoothedMagnitude;
            detectTempoFromPeaks(smoothedMagnitude);
        }
    }
}

//sets sample rate
void TabComponent3::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    this->sampleRate = static_cast<int>(sampleRate);
}

//resource releasing 
void TabComponent3::releaseResources()
{
}
