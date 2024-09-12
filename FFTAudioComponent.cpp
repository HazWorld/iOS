#include "FFTAudioComponent.hpp"

FFTAudioComponent::FFTAudioComponent()
    : forwardFFT(fftOrder), windowingFunction(fftSize, juce::dsp::WindowingFunction<float>::hann)
{
    audioBuffer.resize(fftSize);
    fftData.resize(2 * fftSize);
}

void FFTAudioComponent::initializeAudioResources()
{
    // No need to manually fill the window table; it's handled by JUCE
}

void FFTAudioComponent::processFFTData(const juce::AudioSourceChannelInfo& bufferToFill, std::function<void()> detectionCallback)
{
    if (bufferToFill.buffer->getNumChannels() > 0)
    {
        const float* inputChannelData = bufferToFill.buffer->getReadPointer(0);

        for (int i = 0; i < fftSize; ++i)
        {
            audioBuffer[i] = (i < bufferToFill.buffer->getNumSamples()) ? inputChannelData[i] : 0.0f;
        }

        windowingFunction.multiplyWithWindowingTable(audioBuffer.data(), fftSize);

        std::copy(audioBuffer.begin(), audioBuffer.end(), fftData.begin());
        forwardFFT.performFrequencyOnlyForwardTransform(fftData.data());

        detectionCallback();

        bufferToFill.clearActiveBufferRegion();
    }
}

void FFTAudioComponent::drawGraph(juce::Graphics& g, const juce::Rectangle<float>& bounds)
{
    juce::Path frequencyPath;
    if (fftData.empty()) return;

    const float minFrequency = 20.0f;
    const float maxFrequency = 5000.0f;
    const float minLogFreq = std::log10(minFrequency);
    const float maxLogFreq = std::log10(maxFrequency);

    frequencyPath.startNewSubPath(0, bounds.getHeight());

    const float smoothingFactor = 0.7f;
    float smoothedValue = 0.0f;

    for (int i = 0; i < fftSize / 2; ++i)
    {
        float frequency = i * (sampleRate / fftSize);
        if (frequency < minFrequency || frequency > maxFrequency)
            continue;

        float normalizedX = juce::jmap(std::log10(frequency), minLogFreq, maxLogFreq, 0.0f, bounds.getWidth());

        smoothedValue = (smoothingFactor * fftData[i]) + ((1.0f - smoothingFactor) * smoothedValue);
        float normalizedY = juce::jmap(smoothedValue, 0.0f, 10.0f, bounds.getHeight(), 0.0f);

        if (normalizedY >= 0 && normalizedY <= bounds.getHeight())
            frequencyPath.lineTo(normalizedX, normalizedY);
    }

    frequencyPath.lineTo(bounds.getWidth(), bounds.getHeight());
    frequencyPath.closeSubPath();

    g.setColour(juce::Colours::cyan);
    g.strokePath(frequencyPath, juce::PathStrokeType(2.0f));
}
