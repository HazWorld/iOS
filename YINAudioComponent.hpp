#pragma once

#include <vector>

//==============================================================================
// YIN-based Pitch Detection Class
class YINAudioComponent
{
public:
    YINAudioComponent();

    // Initialize the YIN component with sample rate and buffer size
    void initialize(float sampleRate, int bufferSize);

    // Process the incoming audio buffer and return the detected pitch
    float process(const float* audioBuffer, int bufferSize);

    // Getter for the waveform buffer to be drawn in the GUI
    const std::vector<float>& getWaveformBuffer() const { return waveformBuffer; }

private:
    float tolerance = 0.15f;           // Tolerance for pitch detection threshold
    
    float sampleRate = 0;              // Audio sample rate
    
    std::vector<float> yinBuffer;      // Buffer for YIN difference calculations

    // Buffer to store audio waveform for drawing
    std::vector<float> waveformBuffer;
};
