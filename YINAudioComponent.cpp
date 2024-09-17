#include "YINAudioComponent.hpp"
#include <cmath>
#include <juce_core/juce_core.h>

// Constructor initializing tolerance and buffer size
YINAudioComponent::YINAudioComponent() : tolerance(0.15f), sampleRate(44100.0f) {}

// Initialize the YIN processor with sample rate and buffer size
void YINAudioComponent::initialize(float sampleRate, int bufferSize)
{
    DBG("Initializing YINAudioComponent with sample rate: " + juce::String(sampleRate) + " and buffer size: " + juce::String(bufferSize));
    
    if (sampleRate <= 0 || bufferSize <= 0)
    {
        DBG("Error: Invalid sample rate or buffer size. Initialization failed.");
        return;
    }
    
    this->sampleRate = sampleRate;

    // Resize yinBuffer and waveformBuffer based on bufferSize
    yinBuffer.resize(bufferSize / 2);
    waveformBuffer.resize(bufferSize);  // Store full audio waveform
    DBG("Buffers resized: yinBuffer to " + juce::String(bufferSize / 2) + ", waveformBuffer to " + juce::String(bufferSize));
}

// Process the audio buffer to detect pitch using YIN
float YINAudioComponent::process(const float* audioBuffer, int bufferSize)
{
    if (audioBuffer == nullptr || bufferSize <= 0 || yinBuffer.size() != bufferSize / 2)
    {
        DBG("Invalid audio buffer or buffer size");
        return -1.0f;
    }

    // Copy audioBuffer to waveformBuffer for visualization
    std::copy(audioBuffer, audioBuffer + bufferSize, waveformBuffer.begin());

    // Step 1: Difference function
    for (int tau = 1; tau < bufferSize / 2; tau++)
    {
        yinBuffer[tau] = 0.0f;

        // Optimize by using direct multiplication for difference
        for (int j = 0; (j + tau) < bufferSize && j < bufferSize / 2; j++)
        {
            float diff = audioBuffer[j] - audioBuffer[j + tau];
            yinBuffer[tau] += diff * diff;
        }
    }

    // Step 2: Cumulative mean normalized difference
    float sum = 0.0f;
    const float epsilon = 1e-6f;
    yinBuffer[0] = 1.0f;  // Set the first value to avoid division by zero

    for (int tau = 1; tau < bufferSize / 2; tau++)
    {
        sum += yinBuffer[tau];
        yinBuffer[tau] *= tau / (sum + epsilon);
    }

    // Step 3: Absolute threshold to find pitch
    for (int tau = 1; tau < bufferSize / 2; tau++)
    {
        if (yinBuffer[tau] < tolerance)
        {
            float betterTau = static_cast<float>(tau);
            if (tau > 1 && tau < bufferSize / 2 - 1)
            {
                float s0 = yinBuffer[tau - 1];
                float s1 = yinBuffer[tau];
                float s2 = yinBuffer[tau + 1];
                float denominator = 2.0f * (2.0f * s1 - s2 - s0);

                if (denominator != 0.0f)
                {
                    betterTau += (s2 - s0) / denominator;
                }
            }

            float detectedPitch = sampleRate / betterTau;

            // Limit pitch detection to the typical guitar range (82 Hz to 1318 Hz)
            if (detectedPitch >= 82.41f && detectedPitch <= 1318.51f)
            {
                DBG("Detected pitch: " + juce::String(detectedPitch) + " Hz");
                return detectedPitch;
            }
        }
    }

    DBG("No valid pitch detected.");
    return -1.0f;
}
