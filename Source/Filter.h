// ============================================================================
//  Filter.h  -  resonant state-variable filter (low-pass / high-pass / band-pass)
//
//  This is the well-known "TPT / Cytomic" design.  It stays stable even when
//  the cutoff is swept very fast, which is exactly what rage leads need.
//  A tanh "drive" stage in front makes the filter growl when pushed.
// ============================================================================
#pragma once
#include <cmath>
#include <algorithm>
#include "Oscillator.h"

namespace rl
{
// Fast, smooth saturation curve (close to tanh).
inline float fastTanh(float x)
{
    if (x > 3.0f)  return 1.0f;
    if (x < -3.0f) return -1.0f;
    const float x2 = x * x;
    return x * (27.0f + x2) / (27.0f + 9.0f * x2);
}

struct SvfCoeffs
{
    float a1 = 0, a2 = 0, a3 = 0, k = 1;
};

// cutoffHz: filter frequency, res: 0..1 resonance
inline SvfCoeffs makeSvfCoeffs(float cutoffHz, float res, double sampleRate)
{
    const float maxHz = (float) (sampleRate * 0.45);
    cutoffHz = std::min(std::max(cutoffHz, 15.0f), maxHz);
    const float g = std::tan(kPi * cutoffHz / (float) sampleRate);
    SvfCoeffs c;
    c.k  = 2.0f - 1.95f * std::min(std::max(res, 0.0f), 1.0f); // k = 1/Q
    c.a1 = 1.0f / (1.0f + g * (g + c.k));
    c.a2 = g * c.a1;
    c.a3 = g * c.a2;
    return c;
}

class SvfChannel
{
public:
    void reset() { s1 = s2 = 0.0f; }

    // type: 0 low-pass, 1 high-pass, 2 band-pass.  drive: 0..1
    inline float process(float x, const SvfCoeffs& c, int type, float drive)
    {
        if (drive > 0.001f)
        {
            const float gain = 1.0f + drive * 6.0f;
            x = fastTanh(x * gain) / (1.0f + drive * 1.8f);
        }
        const float v3 = x - s2;
        const float v1 = c.a1 * s1 + c.a2 * v3;
        const float v2 = s2 + c.a2 * s1 + c.a3 * v3;
        s1 = 2.0f * v1 - s1;
        s2 = 2.0f * v2 - s2;

        // keep the state from ever blowing up
        s1 = std::min(std::max(s1, -8.0f), 8.0f);
        s2 = std::min(std::max(s2, -8.0f), 8.0f);

        switch (type)
        {
            case 1:  return x - c.k * v1 - v2;           // high-pass
            case 2:  return v1 * c.k;                    // band-pass (unity-ish gain)
            default: return v2;                          // low-pass
        }
    }

private:
    float s1 = 0.0f, s2 = 0.0f;
};
} // namespace rl
