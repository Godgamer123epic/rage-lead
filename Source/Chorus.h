// ============================================================================
//  Chorus.h  -  3-voice stereo ensemble for width
// ============================================================================
#pragma once
#include <vector>
#include <cmath>
#include "Oscillator.h"

namespace rl
{
class Chorus
{
public:
    void prepare(double sr)
    {
        sampleRate = sr;
        size = (size_t) (sr * 0.06) + 8;
        bufL.assign(size, 0.0f);
        bufR.assign(size, 0.0f);
        reset();
    }

    void reset()
    {
        std::fill(bufL.begin(), bufL.end(), 0.0f);
        std::fill(bufR.begin(), bufR.end(), 0.0f);
        writePos = 0;
        lfoPhase = 0.0;
        mixS = 0.0f;
    }

    void process(float* L, float* R, int n, bool on, float mix, float rateHz)
    {
        const float target = on ? mix : 0.0f;
        const double inc = (double) rateHz / sampleRate;
        const float sr = (float) sampleRate;

        for (int i = 0; i < n; ++i)
        {
            bufL[writePos] = L[i];
            bufR[writePos] = R[i];

            mixS += (target - mixS) * 0.002f;
            float wetL = 0.0f, wetR = 0.0f;

            if (mixS > 0.0002f)
            {
                for (int v = 0; v < 3; ++v)
                {
                    const float phL = (float) lfoPhase + (float) v / 3.0f;
                    const float phR = phL + 0.5f / 3.0f;
                    const float baseMs = 9.0f + 4.0f * (float) v;
                    const float depthMs = 2.6f;
                    const float dL = (baseMs + depthMs * std::sin(kTwoPi * phL)) * 0.001f * sr;
                    const float dR = (baseMs + depthMs * std::sin(kTwoPi * phR + 1.0f)) * 0.001f * sr;
                    wetL += readDelay(bufL, dL);
                    wetR += readDelay(bufR, dR);
                }
                wetL *= (1.0f / 3.0f);
                wetR *= (1.0f / 3.0f);
            }

            L[i] = L[i] * (1.0f - 0.35f * mixS) + wetL * mixS * 0.95f;
            R[i] = R[i] * (1.0f - 0.35f * mixS) + wetR * mixS * 0.95f;

            if (++writePos >= size) writePos = 0;
            lfoPhase += inc;
            if (lfoPhase >= 1.0) lfoPhase -= 1.0;
        }
    }

private:
    float readDelay(const std::vector<float>& buf, float delaySamples) const
    {
        float rp = (float) writePos - delaySamples;
        while (rp < 0.0f) rp += (float) size;
        const size_t i0 = (size_t) rp % size;
        const size_t i1 = (i0 + 1) % size;
        const float fr = rp - std::floor(rp);
        return buf[i0] + (buf[i1] - buf[i0]) * fr;
    }

    double sampleRate = 44100.0;
    size_t size = 1, writePos = 0;
    std::vector<float> bufL, bufR;
    double lfoPhase = 0.0;
    float mixS = 0.0f;
};
} // namespace rl
