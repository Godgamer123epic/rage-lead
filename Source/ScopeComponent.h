// ============================================================================
//  ScopeComponent.h  -  small oscilloscope + spectrum display (click to switch)
// ============================================================================
#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_dsp/juce_dsp.h>
#include "PluginProcessor.h"
#include "LookAndFeel.h"

class ScopeComponent : public juce::Component, public juce::SettableTooltipClient, private juce::Timer
{
public:
    explicit ScopeComponent(RageLeadProcessor& p) : proc(p), fft(11), window(2048, juce::dsp::WindowingFunction<float>::hann)
    {
        setTooltip("Live view of the sound. Click to switch between wave + spectrum / wave only / spectrum only.");
        bars.fill(0.0f);
        ordered.fill(0.0f);
        startTimerHz(30);
    }

    void mouseDown(const juce::MouseEvent&) override
    {
        mode = (mode + 1) % 3;
        repaint();
    }

    void paint(juce::Graphics& g) override
    {
        auto r = getLocalBounds().toFloat();
        g.setColour(juce::Colour(0xff08080b));
        g.fillRoundedRectangle(r, 6.0f);
        g.setColour(rlcol::panelEdge);
        g.drawRoundedRectangle(r.reduced(0.5f), 6.0f, 1.0f);

        auto area = r.reduced(6.0f);

        // centre line
        g.setColour(rlcol::track.withAlpha(0.6f));
        g.drawHorizontalLine((int) area.getCentreY(), area.getX(), area.getRight());

        // spectrum bars
        if (mode != 1)
        {
            const float bw = area.getWidth() / (float) kBars;
            for (int i = 0; i < kBars; ++i)
            {
                const float h = bars[(size_t) i] * area.getHeight();
                g.setColour(rlcol::accent.withAlpha(mode == 2 ? 0.85f : 0.35f));
                g.fillRect(area.getX() + bw * (float) i + 0.5f, area.getBottom() - h, bw - 1.0f, h);
            }
        }

        // waveform
        if (mode != 2)
        {
            juce::Path path;
            const int points = 512;
            for (int i = 0; i < points; ++i)
            {
                const float s = juce::jlimit(-1.0f, 1.0f, wave[(size_t) i]);
                const float x = area.getX() + area.getWidth() * (float) i / (float) (points - 1);
                const float y = area.getCentreY() - s * area.getHeight() * 0.45f;
                if (i == 0) path.startNewSubPath(x, y); else path.lineTo(x, y);
            }
            g.setColour(rlcol::accent.withAlpha(0.25f));
            g.strokePath(path, juce::PathStrokeType(4.0f));
            g.setColour(rlcol::accentHot);
            g.strokePath(path, juce::PathStrokeType(1.6f));
        }
    }

private:
    static constexpr int kBars = 64;

    void timerCallback() override
    {
        // copy the latest 2048 samples in time order
        const int w = proc.scopeWrite.load(std::memory_order_acquire);
        for (int i = 0; i < RageLeadProcessor::kScopeSize; ++i)
            ordered[(size_t) i] = proc.scopeBuf[(size_t) ((w + i) & (RageLeadProcessor::kScopeSize - 1))];

        // oscilloscope: start at a rising zero crossing so the picture holds still
        int start = 0;
        for (int i = 1; i < 1400; ++i)
            if (ordered[(size_t) i - 1] <= 0.0f && ordered[(size_t) i] > 0.0f) { start = i; break; }
        for (int i = 0; i < 512; ++i)
            wave[(size_t) i] = ordered[(size_t) (start + i * 1)];

        // spectrum
        std::fill(fftData.begin(), fftData.end(), 0.0f);
        std::copy(ordered.begin(), ordered.end(), fftData.begin());
        window.multiplyWithWindowingTable(fftData.data(), 2048);
        fft.performFrequencyOnlyForwardTransform(fftData.data());

        const float sr = (float) proc.getSampleRate();
        const float binHz = sr / 2048.0f;
        for (int b = 0; b < kBars; ++b)
        {
            const float f0 = 40.0f * std::pow(16000.0f / 40.0f, (float) b / (float) kBars);
            const float f1 = 40.0f * std::pow(16000.0f / 40.0f, (float) (b + 1) / (float) kBars);
            const int i0 = std::max(1, (int) (f0 / binHz));
            const int i1 = std::min(1023, std::max(i0, (int) (f1 / binHz)));
            float m = 0.0f;
            for (int i = i0; i <= i1; ++i) m = std::max(m, fftData[(size_t) i]);
            const float db = juce::Decibels::gainToDecibels(m / 512.0f, -100.0f);
            const float level = juce::jlimit(0.0f, 1.0f, (db + 70.0f) / 64.0f);
            bars[(size_t) b] = std::max(level, bars[(size_t) b] * 0.85f);
        }
        repaint();
    }

    RageLeadProcessor& proc;
    juce::dsp::FFT fft;
    juce::dsp::WindowingFunction<float> window;
    std::array<float, 2048> ordered {};
    std::array<float, 4096> fftData {};
    std::array<float, 512> wave {};
    std::array<float, kBars> bars {};
    int mode = 0;
};
