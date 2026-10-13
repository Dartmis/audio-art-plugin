#include "PluginProcessor.h"
#include "PluginEditor.h"

AudioARTProcessor::AudioARTProcessor()
    : AudioProcessor (BusesProperties()
                           .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                           .withOutput ("Output", juce::AudioChannelSet::stereo(), true))
{
}

bool AudioARTProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto mono   = juce::AudioChannelSet::mono();
    const auto stereo = juce::AudioChannelSet::stereo();
    const auto set = layouts.getMainOutputChannelSet();
    return set == mono || set == stereo;
}

void AudioARTProcessor::prepareToPlay (double sampleRate, int /*samplesPerBlock*/)
{
    peakRms.prepare (sampleRate);
    loudness.prepare (sampleRate, getTotalNumInputChannels());
    spectrum.prepare (sampleRate);
    goniometer.prepare (sampleRate);
}

void AudioARTProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    const int numSamples = buffer.getNumSamples();
    const int numCh = buffer.getNumChannels();
    if (numSamples == 0) return;

    const float* L = buffer.getReadPointer (0);
    const float* R = numCh > 1 ? buffer.getReadPointer (1) : L;

    // Combined (summed) signal feeds the peak/RMS meter and spectrum, like the HTML version's combineDb().
    static thread_local std::vector<float> combined;
    combined.resize ((size_t) numSamples);
    for (int i = 0; i < numSamples; ++i)
        combined[(size_t) i] = 0.5f * (L[i] + R[i]);

    peakRms.pushBlock (combined.data(), numSamples);
    loudness.pushBlock (L, R, numSamples);
    spectrum.pushSamples (combined.data(), numSamples);
    goniometer.pushBlock (L, R, numSamples, numCh > 1);

    // Pass audio through unmodified -- this is an analyzer, not a processor.
    juce::ignoreUnused (buffer);
}

juce::AudioProcessorEditor* AudioARTProcessor::createEditor()
{
    return new AudioARTEditor (*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AudioARTProcessor();
}
