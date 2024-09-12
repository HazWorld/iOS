
#pragma once

#include <JuceHeader.h>
#include <array>
#include <vector>

//==============================================================================
// Base class for components using FFT processing
class FFTAudioComponent : public juce::Component
{
protected:
    const int fftOrder = 10;
    const int fftSize = 1 << fftOrder;
    std::vector<float> audioBuffer;
    std::vector<float> fftData;
    juce::dsp::FFT forwardFFT;
    juce::dsp::WindowingFunction<float> windowingFunction;
    double sampleRate;

public:
    FFTAudioComponent();
    
    // Common initialization function for windowing
    void initializeAudioResources();
    
    // Common FFT processing method, pass the detection callback for flexibility
    void processFFTData(const juce::AudioSourceChannelInfo& bufferToFill, std::function<void()> detectionCallback);

    // Helper to draw frequency graph (used by both tabs)
    void drawGraph(juce::Graphics& g, const juce::Rectangle<float>& bounds);
};
