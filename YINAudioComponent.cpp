#include "YINAudioComponent.hpp"
#include <cmath>
#include <juce_core/juce_core.h>

// Tweakable parameters
constexpr float DEFAULT_SAMPLE_RATE = 48000.0f;
constexpr float DEFAULT_TOLERANCE = 0.01f;
constexpr float DEFAULT_INPUT_MAGNITUDE_THRESHOLD = 0.000001f;
constexpr float DEFAULT_DYNAMIC_THRESHOLD_MULTIPLIER = 0.005f;
constexpr float FIXED_DYNAMIC_TOLERANCE = 0.05f;  // Static tolerance for low-frequency detection
constexpr int MIN_BUFFER_SIZE = 8192;  // Minimum buffer size for accurate low-frequency detection

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

    // Pre-allocate buffer sizes for performance
    int detectionBufferSize = bufferSize < MIN_BUFFER_SIZE ? MIN_BUFFER_SIZE : bufferSize;
    yinBuffer.resize(detectionBufferSize / 2);
    waveformBuffer.resize(detectionBufferSize);
    accumulatedBuffer.reserve(detectionBufferSize);

    // Precompute Hamming window to avoid recalculating
    hammingWindow.resize(detectionBufferSize);
    for (int i = 0; i < detectionBufferSize; ++i)
    {
        hammingWindow[i] = 0.54f - 0.46f * std::cos(2.0f * juce::MathConstants<float>::pi * i / (detectionBufferSize - 1));
    }

    DBG("Buffers resized: yinBuffer to " + juce::String(detectionBufferSize / 2) + ", waveformBuffer to " + juce::String(detectionBufferSize));
}

void YINAudioComponent::applyHammingWindow(std::vector<float>& buffer)
{
    for (size_t i = 0; i < buffer.size(); ++i)
    {
        buffer[i] *= hammingWindow[i];
    }
}

float YINAudioComponent::processAudioBuffer(const float* audioBuffer, int bufferSize, bool detectMultiplePitches)
{
    // Accumulate the input audio buffer
    accumulatedBuffer.insert(accumulatedBuffer.end(), audioBuffer, audioBuffer + bufferSize);

    // Check if enough samples have been collected
    if (accumulatedBuffer.size() >= yinBuffer.size() * 2)
    {
        int detectionBufferSize = static_cast<int>(yinBuffer.size() * 2);
        std::vector<float> processBuffer(accumulatedBuffer.begin(), accumulatedBuffer.begin() + detectionBufferSize);

        // Choose between single-pitch or multiple-pitch detection
        if (detectMultiplePitches)
        {
            auto detectedPitches = processMultiplePitches(processBuffer.data(), detectionBufferSize);
            if (!detectedPitches.empty())
            {
                DBG("Multiple pitches detected: " + juce::String(detectedPitches.size()));
                // Handle multiple detected pitches
                // This could be returning a vector of detected pitches or further processing
            }
        }
        else
        {
            float detectedPitch = process(processBuffer.data(), detectionBufferSize);
            if (detectedPitch > 0.0f)
            {
                DBG("Single pitch detected: " + juce::String(detectedPitch));
                // Handle the detected pitch
                return detectedPitch;
            }
        }

        // Removing any processed samples
        accumulatedBuffer.erase(accumulatedBuffer.begin(), accumulatedBuffer.begin() + detectionBufferSize);
    }

    return -1.0f; // No pitch detected
}

float YINAudioComponent::process(const float* audioBuffer, int bufferSize)
{
    if (audioBuffer == nullptr || bufferSize <= 0)
        return -1.0f;

    float magnitude = std::accumulate(audioBuffer, audioBuffer + bufferSize, 0.0f, [](float acc, float val) {
        return acc + std::abs(val);
    });

    float dynamicThreshold = std::max(inputMagnitudeThreshold, DEFAULT_DYNAMIC_THRESHOLD_MULTIPLIER * magnitude / bufferSize);
    if (magnitude / bufferSize < dynamicThreshold)
        return -1.0f;

    std::vector<float> windowedBuffer(audioBuffer, audioBuffer + bufferSize);
    applyHammingWindow(windowedBuffer);

    // Auto-correlation for pitch detection
    for (int tau = 1; tau < bufferSize / 2; tau++)
    {
        yinBuffer[tau] = 0.0f;
        for (int j = 0; (j + tau) < bufferSize; j++)
        {
            float diff = windowedBuffer[j] - windowedBuffer[j + tau];
            yinBuffer[tau] += diff * diff;
        }
    }

    // Cumulative mean normalization
    float sum = 0.0f;
    const float epsilon = 1e-6f;
    yinBuffer[0] = 1.0f;

    for (int tau = 1; tau < bufferSize / 2; tau++)
    {
        sum += yinBuffer[tau];
        yinBuffer[tau] *= tau / (sum + epsilon);
    }

    // Detect the first dip
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
                    betterTau += (s2 - s0) / denominator;
            }

            return sampleRate / betterTau;
        }
    }

    return -1.0f;
}


//processes multiple pitches
std::vector<float> YINAudioComponent::processMultiplePitches(const float* audioBuffer, int bufferSize)
{
    std::vector<float> detectedPitches;

    if (audioBuffer == nullptr || bufferSize <= 0)
        return detectedPitches;

    float magnitude = std::accumulate(audioBuffer, audioBuffer + bufferSize, 0.0f, [](float acc, float val) {
        return acc + std::abs(val);
    });

    float dynamicThreshold = std::max(inputMagnitudeThreshold, DEFAULT_DYNAMIC_THRESHOLD_MULTIPLIER * magnitude / bufferSize);
    if (magnitude / bufferSize < dynamicThreshold)
        return detectedPitches;

    std::vector<float> windowedBuffer(audioBuffer, audioBuffer + bufferSize);
    applyHammingWindow(windowedBuffer);

    // Auto-correlation for pitch detection
    for (int tau = 1; tau < bufferSize / 2; tau++)
    {
        yinBuffer[tau] = 0.0f;
        for (int j = 0; (j + tau) < bufferSize; j++)
        {
            float diff = windowedBuffer[j] - windowedBuffer[j + tau];
            yinBuffer[tau] += diff * diff;
        }
    }

    // Cumulative mean normalization
    float sum = 0.0f;
    const float epsilon = 1e-6f;
    yinBuffer[0] = 1.0f;

    for (int tau = 1; tau < bufferSize / 2; tau++)
    {
        sum += yinBuffer[tau];
        yinBuffer[tau] *= tau / (sum + epsilon);
    }

    // Detect multiple dips (pitches)
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
                    betterTau += (s2 - s0) / denominator;
            }

            // Convert to frequency
            float detectedPitch = sampleRate / betterTau;
            detectedPitches.push_back(detectedPitch);
            
            DBG("YIN multiple pitches"+ juce::String(detectedPitch));

            // To avoid detecting the same pitch multiple times, skip over the harmonic range
            tau += static_cast<int>(betterTau * 0.5f);
        }
    }

    return detectedPitches;
}
