#pragma once
#include <juce_dsp/juce_dsp.h>
#include <vector>
#include <cmath>

// Simplified ITU-R BS.1770 K-weighted LUFS meter (Momentary / Short-term /
// Integrated), with the same "freeze until clicked" display behaviour as
// the HTML analyzer. Each of M/S/I has its own independent warm-up gate,
// because Short-term needs ~3s of history before it is meaningful.
class LoudnessMeter
{
public:
    void prepare (double sr, int numChannels)
    {
        sampleRate = sr;
        channels = numChannels;

        // K-weighting: stage 1 high-shelf boost (~ +4dB above 1.5kHz), stage 2 high-pass @ ~38Hz.
        preFilterL.coefficients  = makeKWeightStage1 (sr);
        preFilterR.coefficients  = makeKWeightStage1 (sr);
        shelfFilterL.coefficients = makeKWeightStage2 (sr);
        shelfFilterR.coefficients = makeKWeightStage2 (sr);

        blockMs = 100.0; // 100ms gating blocks per BS.1770
        samplesPerBlock = (int) std::round (sampleRate * blockMs / 1000.0);
        blockAccum.assign ((size_t) samplesPerBlock, 0.0f);
        blockPos = 0;

        reset();
    }

    void reset()
    {
        history.clear();
        integratedSumSq = 0.0;
        integratedCount = 0;
        mWarmMs = sWarmMs = iWarmMs = -1.0;
        mSnap = sSnap = iSnap = -70.0f;
        mLive = sLive = iLive = -70.0f;
        blockPos = 0;
    }

    void resetMomentary()  { mWarmMs = -1.0; mSnap = -70.0f; }
    void resetShortTerm()  { sWarmMs = -1.0; sSnap = -70.0f; }
    void resetIntegrated() { iWarmMs = -1.0; iSnap = -70.0f; history.clear(); integratedSumSq = 0.0; integratedCount = 0; }

    void pushBlock (const float* L, const float* R, int numSamples)
    {
        for (int i = 0; i < numSamples; ++i)
        {
            float l = L[i];
            float r = (channels > 1 ? R[i] : L[i]);

            l = preFilterL.processSample (l);
            l = shelfFilterL.processSample (l);
            r = preFilterR.processSample (r);
            r = shelfFilterR.processSample (r);

            const float p = l * l + r * r; // sum of channel power (K-weighted mean-square proxy)
            blockAccum[(size_t) blockPos] = p;
            if (++blockPos >= samplesPerBlock)
            {
                blockPos = 0;
                double sum = 0.0;
                for (auto v : blockAccum) sum += v;
                const double meanSq = sum / samplesPerBlock;
                const double loudness = -0.691 + 10.0 * std::log10 (meanSq + 1e-12);

                history.push_back (loudness);
                if (history.size() > 30) history.erase (history.begin()); // ~3s @100ms blocks -> momentary window

                mLive = (float) averageLast (400.0);   // 400ms momentary
                sLive = (float) averageLast (3000.0);  // 3s short-term

                if (loudness > -70.0)
                {
                    integratedSumSq += meanSq;
                    integratedCount++;
                    iLive = (float) (-0.691 + 10.0 * std::log10 (integratedSumSq / integratedCount + 1e-12));
                }

                advanceWarm (mWarmMs, mLive > -70.0f, mSnap, mLive, 1200.0);
                advanceWarm (sWarmMs, sLive > -70.0f && history.size() * blockMs >= 3000.0, sSnap, sLive, 1200.0);
                advanceWarm (iWarmMs, integratedCount > 0, iSnap, iLive, 1200.0);
            }
        }
    }

    float getMomentaryLufs()  const { return mSnap; }
    float getShortTermLufs()  const { return sSnap; }
    float getIntegratedLufs() const { return iSnap; }

private:
    double averageLast (double ms)
    {
        const int n = juce::jmax (1, (int) std::round (ms / blockMs));
        const int from = juce::jmax (0, (int) history.size() - n);
        double sumSq = 0.0; int cnt = 0;
        for (size_t i = (size_t) from; i < history.size(); ++i)
        {
            sumSq += std::pow (10.0, (history[i] + 0.691) / 10.0);
            cnt++;
        }
        if (cnt == 0) return -70.0;
        return -0.691 + 10.0 * std::log10 (sumSq / cnt + 1e-12);
    }

    void advanceWarm (double& warmMs, bool valid, float& snap, float live, double warmupMs)
    {
        if (valid && warmMs < 0.0) warmMs = 0.0;
        if (warmMs >= 0.0)
        {
            warmMs += blockMs;
            if (warmMs > warmupMs) snap = live;
        }
    }

    static juce::dsp::IIR::Coefficients<float>::Ptr makeKWeightStage1 (double sr)
    {
        // High-shelf boost ~+4dB above ~1.5kHz (ITU-R BS.1770 pre-filter approximation).
        return juce::dsp::IIR::Coefficients<float>::makeHighShelf (sr, 1500.0, 0.7071, 1.585);
    }
    static juce::dsp::IIR::Coefficients<float>::Ptr makeKWeightStage2 (double sr)
    {
        // RLB weighting curve approximation: high-pass around 38Hz.
        return juce::dsp::IIR::Coefficients<float>::makeHighPass (sr, 38.0, 0.5);
    }

    double sampleRate = 48000.0;
    int channels = 2;
    double blockMs = 100.0;
    int samplesPerBlock = 4800;
    int blockPos = 0;
    std::vector<float> blockAccum;
    std::vector<double> history;
    double integratedSumSq = 0.0;
    long integratedCount = 0;

    double mWarmMs = -1.0, sWarmMs = -1.0, iWarmMs = -1.0;
    float mSnap = -70.0f, sSnap = -70.0f, iSnap = -70.0f;
    float mLive = -70.0f, sLive = -70.0f, iLive = -70.0f;

    juce::dsp::IIR::Filter<float> preFilterL, preFilterR, shelfFilterL, shelfFilterR;
};
