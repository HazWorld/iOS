#pragma once

#include <vector>
#include <juce_core/juce_core.h>

class YINAudioComponent
{
public:
    // Constructor
    YINAudioComponent();

    // Initialization method
    void initialize(float sampleRate, int bufferSize);

    // Main process method for pitch detection
    float process(const float* audioBuffer, int bufferSize);
    
    std::vector<float> processMultiplePitches(const float* audioBuffer, int bufferSize);

    // Apply a Hamming window to the buffer
    void applyHammingWindow(std::vector<float>& buffer);

    // New: Accumulate and process audio buffers
    float processAudioBuffer(const float* audioBuffer, int bufferSize, bool detectMultiplePitches);

private:
    // Buffers for the YIN algorithm and waveform storage
    std::vector<float> yinBuffer;
    std::vector<float> waveformBuffer;
    std::vector<float> accumulatedBuffer;    // Buffer for accumulating samples

    // Precomputed Hamming window
    std::vector<float> hammingWindow;

    // Parameters
    float tolerance;
    float sampleRate;
    float inputMagnitudeThreshold;
};
