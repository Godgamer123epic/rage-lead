// ============================================================================
//  Distortion.h  -  the heart of the rage sound.
//
//  4 modes: Hard Clip, Tube (soft + asymmetric), Wavefold, Bit-Fold.
//  The signal is oversampled 4x while it is distorted, so the harsh edges do
//  not turn into ugly digital aliasing.  A tone filter and a DC blocker follow.
// ============================================================================
#pragma once
#include <juce_dsp/juce_dsp.h>
#include <array>
#include "Filter.h"

namespace rl
{
class Distortion
{
public:
    Distortion() : oversampling(2, 2, juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR, true, false) {}

    void prepare(double sr, int maxBlock)
    {
        sampleRate = sr;
        oversampling.initProcessing((size_t) maxBlock);
        reset();
    }

    void reset()
    {
        oversampling.reset();
        toneL.reset(); toneR.reset();
        dcXL = dcYL = dcXR = dcYR = 0.0f;
        mixS = 0.0f;
    }

    // mode: 0 hard clip, 1 tube, 2 wavefold, 3 bit-fold.  drive & tone: 0..1
    void process(float* L, float* R, int n, bool on, int mode, float drive, float tone)
    {
        const float target = on ? 1.0f : 0.0f;
        if (target < 0.5f && mixS < 0.0005f)
        {
            mixS = 0.0f;
            return;
        }

        drive = std::min(std::max(drive, 0.0f), 1.0f);

        // keep a copy of the clean signal for the on/off crossfade
        for (int i = 0; i < n; ++i) { dryL[(size_t) i] = L[i]; dryR[(size_t) i] = R[i]; }

        float* chans[2] = { L, R };
        juce::dsp::AudioBlock<float> block(chans, 2, (size_t) n);
        auto up = oversampling.processSamplesUp(block);
        const size_t upN = up.getNumSamples();

        const float gain = std::pow(10.0f, drive * 40.0f / 20.0f);       // 0 .. +40 dB
        const float comp = 1.0f / (1.0f + drive * 0.9f);                   // keep loudness in check

        for (size_t ch = 0; ch < 2; ++ch)
        {
            float* d = up.getChannelPointer(ch);
            for (size_t i = 0; i < upN; ++i)
                d[i] = shape(d[i], mode, gain, drive) * comp;
        }
        oversampling.processSamplesDown(block);

        // tone: low-pass sweep (dark ... bright) and DC blocker
        const float toneHz = 700.0f * std::pow(28.0f, std::min(std::max(tone, 0.0f), 1.0f)); // 700 Hz .. ~19 kHz
        const SvfCoeffs tc = makeSvfCoeffs(toneHz, 0.1f, sampleRate);
        for (int i = 0; i < n; ++i)
        {
            float l = toneL.process(L[i], tc, 0, 0.0f);
            float r = toneR.process(R[i], tc, 0, 0.0f);

            const float yl = l - dcXL + 0.999f * dcYL; dcXL = l; dcYL = yl;
            const float yr = r - dcXR + 0.999f * dcYR; dcXR = r; dcYR = yr;

            mixS += (target - mixS) * 0.003f;
            L[i] = dryL[(size_t) i] + (yl - dryL[(size_t) i]) * mixS;
            R[i] = dryR[(size_t) i] + (yr - dryR[(size_t) i]) * mixS;
        }
    }

private:
    static inline float triFold(float x)
    {
        // folds the signal back on itself whenever it goes past +/-1
        return 4.0f * std::abs(0.25f * x + 0.25f - std::round(0.25f * x + 0.25f)) - 1.0f;
    }

    static inline float shape(float x, int mode, float gain, float drive)
    {
        switch (mode)
        {
            case 0:  // Hard clip
                return std::min(std::max(x * gain, -1.0f), 1.0f);
            default:
            case 1:  // Tube: asymmetric soft clip adds even harmonics (warm but loud)
            {
                const float bias = 0.12f;
                return std::tanh(x * gain * 0.7f + bias) - std::tanh(bias);
            }
            case 2:  // Wavefold
                return triFold(x * (1.0f + drive * 14.0f)) * 0.9f;
            case 3:  // Bit-fold: quantise, then fold
            {
                const float levels = 5.0f;
                const float q = std::round(x * (1.0f + drive * 6.0f) * levels) / levels;
                return triFold(q) * 0.9f;
            }
        }
    }

    juce::dsp::Oversampling<float> oversampling;
    double sampleRate = 44100.0;
    SvfChannel toneL, toneR;
    float dcXL = 0, dcYL = 0, dcXR = 0, dcYR = 0;
    float mixS = 0.0f;
    std::array<float, 8192> dryL {}, dryR {};   // enough for the largest block we ever process
};
} // namespace rl
