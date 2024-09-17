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

    // Apply a Hamming window to the buffer
    void applyHammingWindow(float* buffer, int numSamples);

    // New: Accumulate and process audio buffers
    float processAudioBuffer(const float* audioBuffer, int bufferSize);

    // Get the waveform buffer for visualization purposes
    const std::vector<float>& getWaveformBuffer() const { return waveformBuffer; }

    // New: Get the buffer for real-time waveform visualization
    const std::vector<float>& getVisualizationData() const;

private:
    // Buffers for the YIN algorithm and waveform storage
    std::vector<float> yinBuffer;
    std::vector<float> waveformBuffer;
    std::vector<float> visualizationBuffer;  // New: Buffer for visualization
    std::vector<float> accumulatedBuffer;    // New: Buffer for accumulating samples

    // Parameters
    float tolerance;
    float sampleRate;
    float inputMagnitudeThreshold;
};
