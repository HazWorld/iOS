#pragma once

#include <vector>
#include <juce_core/juce_core.h>

class YINAudioComponent
{
public:
    YINAudioComponent();
    void initialize(float sampleRate, int bufferSize);
    float process(const float* audioBuffer, int bufferSize);
    void applyHammingWindow(float* buffer, int numSamples);

    // New: Accumulate the audio buffer
    void processAudioBuffer(const float* audioBuffer, int bufferSize);

    const std::vector<float>& getWaveformBuffer() const { return waveformBuffer; }

private:
    std::vector<float> yinBuffer;
    std::vector<float> waveformBuffer;
    std::vector<float> accumulatedBuffer;  // New: Buffer for accumulating samples

    float tolerance;
    float sampleRate;
    
    float inputMagnitudeThreshold;
};
