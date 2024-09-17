//#pragma once
//
//#include <JuceHeader.h>
//#include <array>
//#include <vector>
//#include <functional>
//
////==============================================================================
//// Base class for components using FFT processing
//class FFTAudioComponent : public juce::Component
//{
//protected:
//    const int fftOrder = 10;           // FFT order (determines FFT size)
//    const int fftSize = 1 << fftOrder; // FFT size (2^fftOrder)
//    
//    std::vector<float> audioBuffer;    // Buffer to hold audio data
//    std::vector<float> fftData;        // FFT data buffer
//    
//    juce::dsp::FFT forwardFFT;         // JUCE's FFT processor
//    juce::dsp::WindowingFunction<float> windowingFunction; // Windowing function to minimize spectral leakage
//    
//    double sampleRate;                 // Sampling rate for audio input
//
//public:
//    FFTAudioComponent();  // Constructor
//    
//    // Initialization function for windowing and other necessary audio resources
//    void initializeAudioResources();
//    
//    // FFT processing method that accepts an audio buffer and a detection callback
//    void processFFTData(const juce::AudioSourceChannelInfo& bufferToFill, std::function<void()> detectionCallback);
//    
//    // Helper method to draw the frequency graph on the UI
//    void drawGraph(juce::Graphics& g, const juce::Rectangle<float>& bounds);
//    
//    // Getter for FFT size
//    int getFFTSize() const { return fftSize; }
//
//    // Getter for sample rate
//    double getSampleRate() const { return sampleRate; }
//    
//    // Utility function to perform parabolic interpolation around the peak index
//    float performParabolicInterpolation(int peakIndex);
//
//private:
//    // Calculates the noise floor dynamically for noise reduction
//    float calculateNoiseFloor();
//};
