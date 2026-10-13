#pragma once
#include <juce_dsp/juce_dsp.h>
#include <atomic>

// Mirrors the "sample-and-hold" peak/RMS behaviour from the HTML analyzer:
// the displayed number auto-settles ~1.2s after valid signal appears, then
// stays frozen until the user clicks it (reset()).
class PeakRmsMeter
{
public:
    void prepare (double sr) { sampleRate = sr; reset(); }

    void pushBlock (const float* data, int numSamples)
    {
        float blockPeak = 0.0f;
        double sumSq = 0.0;
        for (int i = 0; i < numSamples; ++i)
        {
            const float a = std::abs (data[i]);
            blockPeak = juce::jmax (blockPeak, a);
            sumSq += (double) data[i] * (double) data[i];
        }

        livePeakDb = juce::Decibels::gainToDecibels (blockPeak, -100.0f);
        const float blockRms = (float) std::sqrt (sumSq / juce::jmax (1, numSamples));
        liveRmsDb = juce::Decibels::gainToDecibels (blockRms, -100.0f);

        const bool valid = livePeakDb > -90.0f;
        if (valid && warmStartMs < 0.0)
            warmStartMs = 0.0;
        if (warmStartMs >= 0.0)
        {
            warmStartMs += 1000.0 * numSamples / sampleRate;
            if (! snapped && warmStartMs > 1200.0)
            {
                peakSnap = livePeakDb;
                rmsSnap  = liveRmsDb;
                snapped  = true;
            }
        }
        if (! snapped) { peakSnap = livePeakDb; rmsSnap = liveRmsDb; }
    }

    // Called from the message thread when the user clicks the readout.
    void reset()
    {
        warmStartMs = -1.0;
        snapped = false;
        peakSnap = -100.0f;
        rmsSnap  = -100.0f;
    }

    float getPeakDb() const { return peakSnap; }
    float getRmsDb()  const { return rmsSnap; }

private:
    double sampleRate = 48000.0;
    double warmStartMs = -1.0;
    bool snapped = false;
    float livePeakDb = -100.0f, liveRmsDb = -100.0f;
    float peakSnap = -100.0f, rmsSnap = -100.0f;
};
