#include "YINAudioComponent.hpp"
#include <cmath>
#include <juce_core/juce_core.h>

// Tweakable parameters
const float DEFAULT_SAMPLE_RATE = 48000.0f;
const float DEFAULT_TOLERANCE = 0.01f;
const float DEFAULT_INPUT_MAGNITUDE_THRESHOLD = 0.00001f;
const float DEFAULT_DYNAMIC_THRESHOLD_MULTIPLIER = 0.005f;
const float FIXED_DYNAMIC_TOLERANCE = 0.05f;  // Static tolerance for low-frequency detection
const int MIN_BUFFER_SIZE = 8192;  // Minimum buffer size for accurate low-frequency detection

YINAudioComponent::YINAudioComponent()
    : tolerance(DEFAULT_TOLERANCE),
      sampleRate(DEFAULT_SAMPLE_RATE),
      inputMagnitudeThreshold(DEFAULT_INPUT_MAGNITUDE_THRESHOLD) {}

void YINAudioComponent::initialize(float sampleRate, int bufferSize)
{
    DBG("Initializing YINAudioComponent with sample rate: " + juce::String(sampleRate) + " and buffer size: " + juce::String(bufferSize));

    if (sampleRate <= 0 || bufferSize <= 0)
    {
        DBG("Error: Invalid sample rate or buffer size. Initialization failed.");
        return;
    }

    this->sampleRate = sampleRate;

    // Ensure buffer size is at least MIN_BUFFER_SIZE to detect low frequencies
    int detectionBufferSize = bufferSize < MIN_BUFFER_SIZE ? MIN_BUFFER_SIZE : bufferSize;

    if (yinBuffer.size() != detectionBufferSize / 2 || waveformBuffer.size() != detectionBufferSize)
    {
        yinBuffer.resize(detectionBufferSize / 2);
        waveformBuffer.resize(detectionBufferSize);  // Store full audio waveform
        DBG("Buffers resized: yinBuffer to " + juce::String(detectionBufferSize / 2) + ", waveformBuffer to " + juce::String(detectionBufferSize));
    }

    accumulatedBuffer.clear();  // Clear the accumulated buffer on initialization
}

void YINAudioComponent::applyHammingWindow(float* buffer, int numSamples)
{
    for (int i = 0; i < numSamples; ++i)
    {
        float multiplier = 0.54f - 0.46f * std::cos(2.0f * juce::MathConstants<float>::pi * i / (numSamples - 1));
        buffer[i] *= multiplier;
    }
}

// This function now accumulates buffers and only processes when enough samples are gathered
void YINAudioComponent::processAudioBuffer(const float* audioBuffer, int bufferSize)
{
//    DBG("Entering processAudioBuffer method");

    // Accumulate the incoming audio buffer
    accumulatedBuffer.insert(accumulatedBuffer.end(), audioBuffer, audioBuffer + bufferSize);

    // Check if we have enough samples to match the initialized buffer size (e.g., 1024)
    if (accumulatedBuffer.size() >= yinBuffer.size() * 2)
    {
        // Process the accumulated buffer once it's large enough
        std::vector<float> processBuffer(accumulatedBuffer.begin(), accumulatedBuffer.begin() + yinBuffer.size() * 2);

        // Call the pitch detection process with the full buffer
        int detectedPitch = process(processBuffer.data(), static_cast<int>(processBuffer.size()));

        // Remove the processed samples from the accumulated buffer
        accumulatedBuffer.erase(accumulatedBuffer.begin(), accumulatedBuffer.begin() + yinBuffer.size() * 2);

        // Log detected pitch
        if (detectedPitch > 0.0f)
        {
            DBG("Detected pitch: " + juce::String(detectedPitch) + " Hz");
            // You can call a function here to update the UI or handle the pitch result.
        }
    }
}

float YINAudioComponent::process(const float* audioBuffer, int bufferSize)
{
    DBG("Entering process method");

    if (audioBuffer == nullptr || bufferSize <= 0)
    {
        DBG("Invalid audio buffer or buffer size");
        return -1.0f;
    }

    float magnitude = 0.0f;
    for (int i = 0; i < bufferSize; ++i)
    {
        magnitude += std::abs(audioBuffer[i]);
    }
    
    // Dynamic threshold based on signal magnitude
    float dynamicThreshold = std::max(inputMagnitudeThreshold, DEFAULT_DYNAMIC_THRESHOLD_MULTIPLIER * magnitude / bufferSize);
    if (magnitude / bufferSize < dynamicThreshold)
    {
        DBG("Input signal magnitude below threshold, skipping pitch detection.");
        return -1.0f;
    }

    std::vector<float> windowedBuffer(audioBuffer, audioBuffer + bufferSize);
    applyHammingWindow(windowedBuffer.data(), bufferSize);

    std::copy(windowedBuffer.begin(), windowedBuffer.end(), waveformBuffer.begin());

    for (int tau = 1; tau < bufferSize / 2; tau++)
    {
        yinBuffer[tau] = 0.0f;

        for (int j = 0; (j + tau) < bufferSize; j++)
        {
            float diff = windowedBuffer[j] - windowedBuffer[j + tau];
            yinBuffer[tau] += diff * diff;
        }
    }

    float sum = 0.0f;
    const float epsilon = 1e-6f;
    yinBuffer[0] = 1.0f;

    for (int tau = 1; tau < bufferSize / 2; tau++)
    {
        sum += yinBuffer[tau];
        yinBuffer[tau] *= tau / (sum + epsilon);
    }

    // Use fixed dynamic tolerance for pitch detection
    for (int tau = 1; tau < bufferSize / 2; tau++)
    {
        if (yinBuffer[tau] < FIXED_DYNAMIC_TOLERANCE)
        {
            float betterTau = static_cast<float>(tau);
            if (tau > 1 && tau < bufferSize / 2 - 1)
            {
                float s0 = yinBuffer[tau - 1];
                float s1 = yinBuffer[tau];
                float s2 = yinBuffer[tau + 1];
                
                float denominator = 2.0f * (2.0f * s1 - s2 - s0);
                if (std::abs(denominator) > epsilon)
                {
                    betterTau += (s2 - s0) / denominator;
                }
            }

            float detectedPitch = sampleRate / betterTau;

            // Log the detected pitch and tau for debugging
            DBG("Detected tau: " + juce::String(tau) + ", betterTau: " + juce::String(betterTau));
            DBG("Detected pitch: " + juce::String(detectedPitch) + " Hz");

            return detectedPitch;
        }
    }

    DBG("No valid pitch detected.");
    return -1.0f;
}
