#pragma once
#include <juce_core/juce_core.h>
#include <vector>

// Keeps a short ring buffer of L/R sample pairs for the vectorscope
// (goniometer) display, drawn as a single clean connected line per frame
// (no persistence/glow), matching the final HTML analyzer design.
class Goniometer
{
public:
    void prepare (double sr)
    {
        sampleRate = sr;
        const int len = (int) (sr * 0.02); // ~20ms of history per frame, like the HTML version
        buffer.assign ((size_t) juce::jmax (64, len), { 0.0f, 0.0f });
        writePos = 0;
    }

    void pushBlock (const float* L, const float* R, int numSamples, bool stereo)
    {
        for (int i = 0; i < numSamples; ++i)
        {
            buffer[(size_t) writePos] = { L[i], stereo ? R[i] : L[i] };
            writePos = (writePos + 1) % (int) buffer.size();
        }
    }

    const std::vector<std::pair<float, float>>& getBuffer() const { return buffer; }
    int getWritePos() const { return writePos; }

private:
    double sampleRate = 48000.0;
    std::vector<std::pair<float, float>> buffer;
    int writePos = 0;
};
