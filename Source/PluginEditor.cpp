#include "PluginEditor.h"

AudioARTEditor::AudioARTEditor (AudioARTProcessor& p)
    : juce::AudioProcessorEditor (&p), proc (p)
{
    peakLabel.onReset  = [this] { proc.peakRms.reset(); };
    rmsLabel.onReset   = [this] { proc.peakRms.reset(); };
    lufsMLabel.onReset = [this] { proc.loudness.resetMomentary(); };
    lufsSLabel.onReset = [this] { proc.loudness.resetShortTerm(); };
    lufsILabel.onReset = [this] { proc.loudness.resetIntegrated(); };

    for (auto* c : { (juce::Component*) &peakLabel, &rmsLabel, &lufsMLabel, &lufsSLabel, &lufsILabel })
        addAndMakeVisible (c);

    setSize (780, 520);
    startTimerHz (30);
}

AudioARTEditor::~AudioARTEditor() { stopTimer(); }

void AudioARTEditor::resized()
{
    auto area = getLocalBounds().reduced (12);
    auto meterRow = area.removeFromTop (54);
    const int w = meterRow.getWidth() / 5;
    peakLabel.setBounds  (meterRow.removeFromLeft (w).reduced (4));
    rmsLabel.setBounds   (meterRow.removeFromLeft (w).reduced (4));
    lufsMLabel.setBounds (meterRow.removeFromLeft (w).reduced (4));
    lufsSLabel.setBounds (meterRow.removeFromLeft (w).reduced (4));
    lufsILabel.setBounds (meterRow.removeFromLeft (w).reduced (4));
}

void AudioARTEditor::timerCallback()
{
    peakLabel.setValue (proc.peakRms.getPeakDb(), "dB");
    rmsLabel.setValue  (proc.peakRms.getRmsDb(),  "dB");
    lufsMLabel.setValue (proc.loudness.getMomentaryLufs(),  "LUFS");
    lufsSLabel.setValue (proc.loudness.getShortTermLufs(),  "LUFS");
    lufsILabel.setValue (proc.loudness.getIntegratedLufs(), "LUFS");
    proc.spectrum.computeIfReady();
    repaint();
}

void AudioARTEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff0a0a0e));

    auto area = getLocalBounds().reduced (12);
    area.removeFromTop (54); // meter row handled by child components

    auto left  = area.removeFromLeft (area.getWidth() * 2 / 3).reduced (6);
    auto right = area.reduced (6);

    paintSpectrum (g, left);
    paintGoniometer (g, right);
}

void AudioARTEditor::paintSpectrum (juce::Graphics& g, juce::Rectangle<int> area)
{
    g.setColour (juce::Colour (0xff14141c));
    g.fillRoundedRectangle (area.toFloat(), 8.0f);
    g.setColour (juce::Colour (0xff8fa3c7));
    g.setFont (12.0f);
    g.drawText ("SPECTRUM", area.removeFromTop (20), juce::Justification::centred);

    const auto& bands = proc.spectrum.getBandsDb();
    const int n = (int) bands.size();
    if (n == 0 || area.getWidth() < 10) return;

    const float plotMinDb = -100.0f, plotMaxDb = 6.0f;
    juce::Path p;
    for (int i = 0; i < n; ++i)
    {
        const float x = area.getX() + area.getWidth() * (float) i / (float) (n - 1);
        const float t = juce::jlimit (0.0f, 1.0f, (bands[(size_t) i] - plotMinDb) / (plotMaxDb - plotMinDb));
        const float y = area.getBottom() - t * area.getHeight();
        if (i == 0) p.startNewSubPath (x, y); else p.lineTo (x, y);
    }
    g.setColour (juce::Colour (0xffd8c8ff));
    g.strokePath (p, juce::PathStrokeType (1.5f));
}

void AudioARTEditor::paintGoniometer (juce::Graphics& g, juce::Rectangle<int> area)
{
    g.setColour (juce::Colour (0xff14141c));
    g.fillRoundedRectangle (area.toFloat(), 8.0f);
    g.setColour (juce::Colour (0xff8fa3c7));
    g.setFont (12.0f);
    g.drawText ("VECTOR", area.removeFromTop (20), juce::Justification::centred);

    const auto& buf = proc.goniometer.getBuffer();
    if (buf.empty() || area.getWidth() < 10) return;

    const auto cx = (float) area.getCentreX();
    const auto cy = (float) area.getCentreY();
    const auto radius = (float) juce::jmin (area.getWidth(), area.getHeight()) * 0.45f;

    juce::Path p;
    bool first = true;
    for (size_t i = 0; i < buf.size(); ++i)
    {
        const auto [l, r] = buf[i];
        const float mid = (l + r) * 0.70710678f;
        const float side = (l - r) * 0.70710678f;
        const float x = cx + side * radius;
        const float y = cy - mid * radius;
        if (first) { p.startNewSubPath (x, y); first = false; } else p.lineTo (x, y);
    }
    g.setColour (juce::Colour (0xff8fc4ff));
    g.strokePath (p, juce::PathStrokeType (1.0f));
}
