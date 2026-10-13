#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"

// Dark-themed panel showing a numeric readout that freezes until clicked,
// mirroring the HTML analyzer's meter behaviour.
class ClickToResetLabel : public juce::Component
{
public:
    ClickToResetLabel (juce::String labelText, std::function<void()> onResetFn)
        : text (std::move (labelText)), onReset (std::move (onResetFn)) {}

    void setValue (float db, const juce::String& unitStr)
    {
        value = db;
        unit = unitStr;
        repaint();
    }

    void paint (juce::Graphics& g) override
    {
        auto b = getLocalBounds().toFloat();
        g.setColour (juce::Colour (0xff1a1a22));
        g.fillRoundedRectangle (b, 6.0f);
        g.setColour (juce::Colour (0xff8fa3c7));
        g.setFont (juce::Font (11.0f));
        g.drawText (text, b.removeFromTop (16.0f).reduced (6, 0), juce::Justification::centredLeft);
        g.setColour (juce::Colour (0xffe8e8f0));
        g.setFont (juce::Font (20.0f, juce::Font::bold));
        juce::String s = value <= -99.0f ? juce::String ("-∞") : juce::String (value, 1);
        g.drawText (s + " " + unit, b.reduced (6, 0), juce::Justification::centredLeft);
    }

    void mouseUp (const juce::MouseEvent&) override { if (onReset) onReset(); }

    std::function<void()> onReset;

private:
    juce::String text, unit = "dB";
    float value = -100.0f;
};

class AudioARTEditor final : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit AudioARTEditor (AudioARTProcessor&);
    ~AudioARTEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;
    void paintSpectrum (juce::Graphics&, juce::Rectangle<int> area);
    void paintGoniometer (juce::Graphics&, juce::Rectangle<int> area);

    AudioARTProcessor& proc;

    ClickToResetLabel peakLabel  { "PEAK", nullptr };
    ClickToResetLabel rmsLabel   { "RMS",  nullptr };
    ClickToResetLabel lufsMLabel { "LUFS-M", nullptr };
    ClickToResetLabel lufsSLabel { "LUFS-S", nullptr };
    ClickToResetLabel lufsILabel { "LUFS-I", nullptr };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioARTEditor)
};
