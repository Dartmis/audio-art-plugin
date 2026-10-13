#pragma once
#include <juce_dsp/juce_dsp.h>
#include <array>

// FFT-based spectrum analyzer feeding a set of log-spaced band magnitudes,
// mirroring the HTML analyzer's BAND_CENTERS approach.
class SpectrumAnalyzer
{
public:
    static constexpr int fftOrder = 11;         // 2048 points
    static constexpr int fftSize  = 1 << fftOrder;
    static constexpr int numBands = 48;

    SpectrumAnalyzer() : fft (fftOrder) {}

    void prepare (double sr)
    {
        sampleRate = sr;
        fifoIndex = 0;
        nextFftBlockReady = false;
        fifo.fill (0.0f);
        fftData.fill (0.0f);
        bandsDb.fill (-100.0f);

        // Log-spaced band centers from 20Hz to Nyquist-ish 20kHz.
        for (int b = 0; b < numBands; ++b)
        {
            const double t = (double) b / (double) (numBands - 1);
            bandCenters[(size_t) b] = 20.0 * std::pow (1000.0, t); // 20 .. 20000 Hz
        }
    }

    void pushSamples (const float* data, int numSamples)
    {
        for (int i = 0; i < numSamples; ++i)
        {
            if (fifoIndex == fftSize)
            {
                if (! nextFftBlockReady)
                {
                    std::fill (fftData.begin(), fftData.end(), 0.0f);
                    std::copy (fifo.begin(), fifo.end(), fftData.begin());
                    nextFftBlockReady = true;
                }
                fifoIndex = 0;
            }
            fifo[(size_t) fifoIndex++] = data[i];
        }
    }

    // Call once per UI refresh from the message thread.
    bool computeIfReady()
    {
        if (! nextFftBlockReady) return false;
        window.multiplyWithWindowingTable (fftData.data(), fftSize);
        fft.performFrequencyOnlyForwardTransform (fftData.data());

        const double binHz = sampleRate / fftSize;
        for (int b = 0; b < numBands; ++b)
        {
            const double lo = b == 0 ? bandCenters[0] * 0.8 : std::sqrt (bandCenters[(size_t) b - 1] * bandCenters[(size_t) b]);
            const double hi = b == numBands - 1 ? bandCenters[(size_t) numBands - 1] * 1.2
                                                 : std::sqrt (bandCenters[(size_t) b] * bandCenters[(size_t) b + 1]);
            int binLo = juce::jmax (1, (int) std::floor (lo / binHz));
            int binHi = juce::jmin (fftSize / 2, (int) std::ceil (hi / binHz));
            double sum = 0.0; int cnt = 0;
            for (int k = binLo; k <= binHi; ++k) { sum += fftData[(size_t) k]; cnt++; }
            const double mag = cnt > 0 ? sum / cnt : 0.0;
            const double db = 20.0 * std::log10 (mag + 1e-9) ;
            bandsDb[(size_t) b] = (float) juce::jlimit (-100.0, 6.0, db);
        }
        nextFftBlockReady = false;
        return true;
    }

    const std::array<float, numBands>& getBandsDb() const { return bandsDb; }
    const std::array<double, numBands>& getBandCenters() const { return bandCenters; }

private:
    double sampleRate = 48000.0;
    juce::dsp::FFT fft;
    juce::dsp::WindowingFunction<float> window { (size_t) fftSize, juce::dsp::WindowingFunction<float>::hann };

    std::array<float, (size_t) fftSize * 2> fftData {};
    std::array<float, (size_t) fftSize> fifo {};
    int fifoIndex = 0;
    bool nextFftBlockReady = false;

    std::array<float, numBands> bandsDb {};
    std::array<double, numBands> bandCenters {};
};
