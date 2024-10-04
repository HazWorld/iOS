#pragma once

#include <vector>
#include <juce_core/juce_core.h>

class YINAudioComponent
{
public:

    YINAudioComponent();

    void initialize(float sampleRate, int bufferSize);

    float process(const float* audioBuffer, int bufferSize);

    float processAudioBuffer(const float* audioBuffer, int bufferSize);

    void applyHammingWindow(std::vector<float>& buffer);

private:

    std::vector<float> yinBuffer;

    std::vector<float> accumulatedBuffer;

    std::vector<float> hammingWindow;

    float tolerance;
    float sampleRate;
    float inputMagnitudeThreshold;
};
