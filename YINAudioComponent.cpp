#include "YINAudioComponent.hpp"
#include <cmath>
#include <juce_core/juce_core.h>

//tweakable parameters
const float DEFAULT_SAMPLE_RATE = 48000.0f;
const float DEFAULT_TOLERANCE = 0.01f;
const float DEFAULT_INPUT_MAGNITUDE_THRESHOLD = 0.05f;
const float DEFAULT_DYNAMIC_THRESHOLD_MULTIPLIER = 0.005f;
const float FIXED_DYNAMIC_TOLERANCE = 0.05f;  // Static tolerance for low-frequency detection
const int MIN_BUFFER_SIZE = 8192;  // Minimum buffer size for accurate low-frequency detection

YINAudioComponent::YINAudioComponent()
    : tolerance(DEFAULT_TOLERANCE),
      sampleRate(DEFAULT_SAMPLE_RATE),
      inputMagnitudeThreshold(DEFAULT_INPUT_MAGNITUDE_THRESHOLD) {}


//initializer for the YIN processor
//this sets sample rate, buffer sizes and presets the hamming window
void YINAudioComponent::initialize(float sampleRate, int bufferSize)
{
    DBG("Initializing with sample rate: " + juce::String(sampleRate) + " and buffer size: " + juce::String(bufferSize));

    if (sampleRate <= 0 || bufferSize <= 0)
    {
        DBG("Invalid sample rate or buffer size- initialization failed.");
        return;
    }

    this->sampleRate = sampleRate;

    //allocate buffer sizes
    int detectionBufferSize = bufferSize < MIN_BUFFER_SIZE ? MIN_BUFFER_SIZE : bufferSize;
    yinBuffer.resize(detectionBufferSize / 2);
    accumulatedBuffer.reserve(detectionBufferSize);

    //precompute Hamming window to avoid recalculating
    hammingWindow.resize(detectionBufferSize);
    for (int i = 0; i < detectionBufferSize; ++i)
    {
        hammingWindow[i] = 0.54f - 0.46f * std::cos(2.0f * juce::MathConstants<float>::pi * i / (detectionBufferSize - 1));
    }

}

//apply hamming window to signal
void YINAudioComponent::applyHammingWindow(std::vector<float>& buffer)
{
    for (size_t i = 0; i < buffer.size(); ++i)
    {
        buffer[i] *= hammingWindow[i];
    }
}

//Handles the accumulated buffer required for YIN processing and applys yin processing
float YINAudioComponent::processAudioBuffer(const float* audioBuffer, int bufferSize)
{
    //starts accumulated buffer
    accumulatedBuffer.insert(accumulatedBuffer.end(), audioBuffer, audioBuffer + bufferSize);

    //check accumulated buffer size meets the required buffer
    if (accumulatedBuffer.size() >= yinBuffer.size() * 2)
    {
        
        int detectionBufferSize = static_cast<int>(yinBuffer.size() * 2);
        std::vector<float> processBuffer(accumulatedBuffer.begin(), accumulatedBuffer.begin() + detectionBufferSize);

        //passes signal through the YIN process
        float detectedPitch = process(processBuffer.data(), detectionBufferSize);
        if (detectedPitch > 0.0f)
        {
            DBG("Pitch detected: " + juce::String(detectedPitch));
            accumulatedBuffer.clear();  //clears the buffer
            return detectedPitch;
        }

        //remove processed samples
        accumulatedBuffer.erase(accumulatedBuffer.begin(), accumulatedBuffer.begin() + detectionBufferSize);
    }

    return -1.0f; //no pitch detected
}

float YINAudioComponent::process(const float* audioBuffer, int bufferSize)
{
    //checks for audio buffer
    if (audioBuffer == nullptr || bufferSize <= 0)
        return -1.0f;

    //calculates the magnitude of the buffer
    float magnitude = std::accumulate(audioBuffer, audioBuffer + bufferSize, 0.0f, [](float acc, float val) {
        return acc + std::abs(val);
    });

    //sets dynamic threshold
    float dynamicThreshold = std::max(inputMagnitudeThreshold, DEFAULT_DYNAMIC_THRESHOLD_MULTIPLIER * magnitude / bufferSize);
    //checks if magnitude of inpuit signal is below the threshold
    if (magnitude / bufferSize < dynamicThreshold)
        return -1.0f;

    //application of the hamming windowing
    std::vector<float> windowedBuffer(audioBuffer, audioBuffer + bufferSize);
    applyHammingWindow(windowedBuffer);

    //auto correlation
    for (int tau = 1; tau < bufferSize / 2; tau++)
    {
        yinBuffer[tau] = 0.0f;
        for (int j = 0; (j + tau) < bufferSize; j++)
        {
            float diff = windowedBuffer[j] - windowedBuffer[j + tau];
            yinBuffer[tau] += diff * diff;
        }
    }

    
    //cumulative mean normalization
    float sum = 0.0f;
    const float epsilon = 1e-6f; //prevent division by 0 to avoid errors
    
    yinBuffer[0] = 1.0f;
    for (int tau = 1; tau < bufferSize / 2; tau++)
    {
        sum += yinBuffer[tau]; //gather differences
        yinBuffer[tau] *= tau / (sum + epsilon); //normalisation
    }

    //detect the first dip
    for (int tau = 1; tau < bufferSize / 2; tau++)
    {
        if (yinBuffer[tau] < FIXED_DYNAMIC_TOLERANCE)
        {
            float betterTau = static_cast<float>(tau); //initial tau

            //refine better tau using parabolic interpolation
            if (tau > 1 && tau < bufferSize / 2 - 1)
            {
                float s0 = yinBuffer[tau - 1];
                float s1 = yinBuffer[tau];
                float s2 = yinBuffer[tau + 1];

                float denominator = 2.0f * (2.0f * s1 - s2 - s0);
                if (std::abs(denominator) > epsilon)
                    betterTau += (s2 - s0) / denominator; //set new betterTau
            }

            return sampleRate / betterTau; //returns the pitch detected
        }
    }

    return -1.0f;
}
