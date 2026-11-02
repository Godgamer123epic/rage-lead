// ============================================================================
//  Limiter.h  -  output stage: master volume, limiter and a safety soft-clipper
//
//  The limiter turns the volume down automatically when the sound gets too
//  loud, and the soft clipper after it guarantees the output NEVER goes over
//  0 dBFS, whatever the knobs are doing.
// ============================================================================
#pragma once
#include <cmath>
#include <algorithm>

namespace rl
{
class OutputStage
{
public:
    void prepare(double sr)
    {
        sampleRate = sr;
        attackCoef  = 1.0f - std::exp(-1.0f / (0.0004f * (float) sr));
        releaseCoef = 1.0f - std::exp(-1.0f / (0.120f * (float) sr));
        reset();
    }

    void reset()
    {
        env = 0.0f;
        gain = 1.0f;
        volume = -1.0f;
    }

    void process(float* L, float* R, int n, float masterDb, bool limiterOn)
    {
        const float targetVol = std::pow(10.0f, masterDb / 20.0f);
        if (volume < 0.0f) volume = targetVol;
        const float threshold = 0.89f; // about -1 dB

        for (int i = 0; i < n; ++i)
        {
            volume += (targetVol - volume) * 0.002f;
            float l = L[i] * volume;
            float r = R[i] * volume;

            if (limiterOn)
            {
                const float peak = std::max(std::abs(l), std::abs(r));
                env = std::max(peak, env + (peak - env) * releaseCoef);
                const float wanted = env > threshold ? threshold / env : 1.0f;
                gain += (wanted - gain) * (wanted < gain ? attackCoef : releaseCoef);
                // never let the gain lag behind a big peak
                if (peak * gain > threshold * 1.5f) gain = threshold / peak;
                l *= gain;
                r *= gain;
            }
            L[i] = softClip(l);
            R[i] = softClip(r);
        }
    }

    // peak-safe soft clipper: linear below 0.9, smoothly bends towards 1.0 above it
    static inline float softClip(float x)
    {
        const float a = std::abs(x);
        if (a < 0.9f) return x;
        const float y = 0.9f + 0.1f * std::tanh((a - 0.9f) / 0.1f);
        return x < 0.0f ? -y : y;
    }

private:
    double sampleRate = 44100.0;
    float attackCoef = 0.1f, releaseCoef = 0.001f;
    float env = 0.0f, gain = 1.0f, volume = -1.0f;
};
} // namespace rl
