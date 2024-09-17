//#include "FFTAudioComponent.hpp"
//
//FFTAudioComponent::FFTAudioComponent()
//    : forwardFFT(fftOrder),
//      windowingFunction(fftSize, juce::dsp::WindowingFunction<float>::hann)
//{
//    audioBuffer.resize(fftSize);
//    fftData.resize(2 * fftSize);  // FFT data is twice the size of the FFT
//}
//
//void FFTAudioComponent::initializeAudioResources()
//{
//    // Set up any additional resources you may need, such as filters or extra buffers
//}
//
//void FFTAudioComponent::processFFTData(const juce::AudioSourceChannelInfo& bufferToFill, std::function<void()> detectionCallback)
//{
//    if (bufferToFill.buffer->getNumChannels() > 0)
//    {
//        const float* inputChannelData = bufferToFill.buffer->getReadPointer(0);
//        int numSamples = bufferToFill.buffer->getNumSamples();
//
//        // Zero-padding for unfilled audio buffer regions
//        juce::FloatVectorOperations::clear(audioBuffer.data() + numSamples, fftSize - numSamples);
//
//        // Copy as much as we can from the input buffer
//        juce::FloatVectorOperations::copy(audioBuffer.data(), inputChannelData, juce::jmin(fftSize, numSamples));
//
//        // Apply windowing function to the audio buffer
//        windowingFunction.multiplyWithWindowingTable(audioBuffer.data(), fftSize);
//
//        // Perform the FFT on the windowed buffer
//        std::copy(audioBuffer.begin(), audioBuffer.end(), fftData.begin());
//        forwardFFT.performFrequencyOnlyForwardTransform(fftData.data());
//
//        // Apply noise thresholding
//        const float noiseFloor = calculateNoiseFloor();
//        const float dynamicThreshold = noiseFloor * 2.0f;
//        for (int i = 0; i < fftSize / 2; ++i)
//        {
//            if (fftData[i] < dynamicThreshold)
//            {
//                fftData[i] = 0.0f;
//            }
//        }
//
//        // Execute detection callback
//        detectionCallback();
//    }
//}
//
//float FFTAudioComponent::calculateNoiseFloor()
//{
//    float sumNoise = 0.0f;
//    int count = 0;
//
//    // Average the lower magnitude frequencies to determine the noise floor
//    for (int i = 1; i < fftSize / 4; ++i)  // Only consider lower frequencies for noise
//    {
//        sumNoise += fftData[i];
//        count++;
//    }
//
//    return count > 0 ? (sumNoise / count) : 0.0f;
//}
//
//float FFTAudioComponent::performParabolicInterpolation(int peakIndex)
//{
//    if (peakIndex <= 0 || peakIndex >= (fftSize / 2) - 1)
//        return peakIndex;  // Cannot interpolate on edges
//
//    // Get the magnitudes of the current peak and its neighbors
//    float alpha = fftData[peakIndex - 1];
//    float beta = fftData[peakIndex];
//    float gamma = fftData[peakIndex + 1];
//
//    // Parabolic interpolation formula
//    float correction = (gamma - alpha) / (2.0f * (2.0f * beta - alpha - gamma));
//
//    // Return the interpolated peak index
//    return peakIndex + correction;
//}
//
//
//
//void FFTAudioComponent::drawGraph(juce::Graphics& g, const juce::Rectangle<float>& bounds)
//{
//    juce::Path frequencyPath;
//    if (fftData.empty()) return;
//
//    // Adjusted frequency ranges to emphasize the mid-range more
//    const float minFrequency = 50.0f;    // Raised minimum frequency to reduce low-end
//    const float maxFrequency = 10000.0f; // Max frequency kept at 10kHz to emphasize mids
//
//    const float minLogFreq = std::log10(minFrequency);
//    const float maxLogFreq = std::log10(maxFrequency);
//
//    frequencyPath.startNewSubPath(0, bounds.getHeight());
//
//    // Smoothing factors for rise and decay
//    const float riseFactor = 0.9f;  // Fast rise
//    const float decayFactor = 0.8f;  // Slow decay
//    static std::vector<float> smoothedValues(fftSize / 2, 0.0f); // Store previous smoothed values
//
//    int highestFreqIndex = -1;
//    float maxMagnitude = 0.0f;
//
//    // Loop through the FFT data and map the frequencies/logarithmic scaling
//    for (int i = 0; i < fftSize / 2; ++i)
//    {
//        float frequency = i * (sampleRate / fftSize);
//        if (frequency < minFrequency || frequency > maxFrequency)
//            continue;
//
//        // Check for highest magnitude to find the peak frequency
//        if (fftData[i] > maxMagnitude)
//        {
//            maxMagnitude = fftData[i];
//            highestFreqIndex = i;
//        }
//
//        // Use logarithmic mapping to emphasize the mid-range frequencies
//        float normalizedX = juce::jmap(std::log10(frequency), minLogFreq, maxLogFreq, 0.0f, bounds.getWidth());
//
//        // Update the smoothed value based on rise or decay
//        if (fftData[i] > smoothedValues[i])
//        {
//            // Fast rise when the incoming data is greater than the current smoothed value
//            smoothedValues[i] = (riseFactor * fftData[i]) + ((1.0f - riseFactor) * smoothedValues[i]);
//        }
//        else
//        {
//            // Slow decay when the incoming data is less than the current smoothed value
//            smoothedValues[i] = (decayFactor * smoothedValues[i]) + ((1.0f - decayFactor) * fftData[i]);
//        }
//
//        // Convert the smoothed value to a normalized Y value for graph display
//        float normalizedY = juce::jmap(smoothedValues[i], 0.0f, 10.0f, bounds.getHeight(), 0.0f);
//
//        if (normalizedY >= 0 && normalizedY <= bounds.getHeight())
//            frequencyPath.lineTo(normalizedX, normalizedY);
//    }
//
//    // Add a final line back to the bottom of the graph (closing the response area)
//    frequencyPath.lineTo(bounds.getWidth(), bounds.getHeight());
//    frequencyPath.closeSubPath();
//
//    // Draw the response curve with a gradient
//    juce::ColourGradient gradient(juce::Colours::cyan, 0, 0, juce::Colours::green, bounds.getWidth(), bounds.getHeight(), false);
//    g.setGradientFill(gradient);
//    g.strokePath(frequencyPath, juce::PathStrokeType(2.5f));
//
//    // Optional: Draw border around the graph
//    g.setColour(juce::Colours::grey);
//    g.drawRect(bounds);
//
//    // Display the highest frequency
//    if (highestFreqIndex != -1)
//    {
//        float highestFrequency = highestFreqIndex * (sampleRate / fftSize);
//        g.setColour(juce::Colours::white);
//        g.setFont(15.0f);
//
//        // Create a mutable copy of the bounds rectangle
//        juce::Rectangle<float> mutableBounds = bounds;
//
//        juce::String frequencyText = "Highest Frequency: " + juce::String(highestFrequency, 2) + " Hz";
//        g.drawText(frequencyText, mutableBounds.removeFromBottom(30), juce::Justification::centred);
//    }
//}
//
//
