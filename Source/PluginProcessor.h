#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "Meters.h"
#include "LoudnessMeter.h"
#include "SpectrumAnalyzer.h"
#include "Goniometer.h"

class AudioARTProcessor final : public juce::AudioProcessor
{
public:
    AudioARTProcessor();
    ~AudioARTProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "Audio ART"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock&) override {}
    void setStateInformation (const void*, int) override {}

    // Shared analysis state, read by the editor (UI) thread for display.
    PeakRmsMeter peakRms;
    LoudnessMeter loudness;
    SpectrumAnalyzer spectrum;
    Goniometer goniometer;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioARTProcessor)
};
