// ============================================================================
//  Reverb.h  -  big dark reverb with pre-delay and an optional "gate"
//
//  The gate cuts the reverb tail abruptly a short time after the notes stop -
//  the classic chopped 80s/rage tail.
// ============================================================================
#pragma once
#include <juce_audio_basics/juce_audio_basics.h>
#include <vector>
#include "Filter.h"

namespace rl
{
class DarkReverb
{
public:
    void prepare(double sr, int maxBlock)
    {
        sampleRate = sr;
        preSize = (size_t) (sr * 0.2) + 8;
        preL.assign(preSize, 0.0f);
        preR.assign(preSize, 0.0f);
        tmpL.assign((size_t) maxBlock, 0.0f);
        tmpR.assign((size_t) maxBlock, 0.0f);
        reverb.setSampleRate(sr);
        reset();
    }

    void reset()
    {
        reverb.reset();
        std::fill(preL.begin(), preL.end(), 0.0f);
        std::fill(preR.begin(), preR.end(), 0.0f);
        prePos = 0;
        mixS = 0.0f;
        gateGain = 1.0f;
        holdCounter = 0;
        inEnv = 0.0f;
        darkL.reset(); darkR.reset();
    }

    void process(float* L, float* R, int n, bool on, float size, float darkness, float preMs, float mix,
                 bool gateOn, float gateMs)
    {
        // configure the reverb engine
        juce::Reverb::Parameters rp;
        rp.roomSize = 0.55f + 0.44f * size;
        rp.damping  = 0.25f + 0.7f * darkness;
        rp.wetLevel = 0.33f;
        rp.dryLevel = 0.0f;
        rp.width    = 1.0f;
        rp.freezeMode = 0.0f;
        reverb.setParameters(rp);

        // pre-delay
        const size_t preSamples = std::min((size_t) (preMs * 0.001f * (float) sampleRate), preSize - 2);
        for (int i = 0; i < n; ++i)
        {
            preL[prePos] = L[i];
            preR[prePos] = R[i];
            const size_t rd = (prePos + preSize - preSamples) % preSize;
            tmpL[(size_t) i] = preL[rd];
            tmpR[(size_t) i] = preR[rd];
            if (++prePos >= preSize) prePos = 0;
        }

        reverb.processStereo(tmpL.data(), tmpR.data(), n);

        // make the tail darker
        const float darkHz = 14000.0f * std::pow(0.17f, darkness);   // 14 kHz ... ~2.4 kHz
        const SvfCoeffs dc = makeSvfCoeffs(darkHz, 0.05f, sampleRate);
        const float target = on ? mix : 0.0f;
        const int holdSamples = (int) (gateMs * 0.001f * (float) sampleRate);
        const float gateCoef = 1.0f - std::exp(-1.0f / (0.006f * (float) sampleRate));

        for (int i = 0; i < n; ++i)
        {
            // gate: opens while there is input, closes shortly after
            const float level = std::max(std::abs(L[i]), std::abs(R[i]));
            inEnv = std::max(level, inEnv * 0.9995f);
            if (inEnv > 0.02f) holdCounter = holdSamples;
            else if (holdCounter > 0) --holdCounter;
            const float gateTarget = (! gateOn || holdCounter > 0) ? 1.0f : 0.0f;
            gateGain += (gateTarget - gateGain) * gateCoef;

            mixS += (target - mixS) * 0.002f;

            const float wl = darkL.process(tmpL[(size_t) i], dc, 0, 0.0f) * gateGain * mixS * 1.6f;
            const float wr = darkR.process(tmpR[(size_t) i], dc, 0, 0.0f) * gateGain * mixS * 1.6f;
            L[i] += wl;
            R[i] += wr;
        }
    }

private:
    juce::Reverb reverb;
    double sampleRate = 44100.0;
    size_t preSize = 1, prePos = 0;
    std::vector<float> preL, preR, tmpL, tmpR;
    float mixS = 0.0f, gateGain = 1.0f, inEnv = 0.0f;
    int holdCounter = 0;
    SvfChannel darkL, darkR;
};
} // namespace rl
