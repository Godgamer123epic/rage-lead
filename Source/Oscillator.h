// ============================================================================
//  Oscillator.h  -  the raw sound sources.
//
//  Every function takes the oscillator "phase" (0..1, where in the wave cycle
//  we are) and "dt" (how far the phase moves each sample = frequency / sample
//  rate) and returns one sample.  Saw and pulse waves use "PolyBLEP", a cheap
//  trick that smooths the sharp edges so they do not alias (no ugly fizz).
// ============================================================================
#pragma once
#include <cmath>
#include <algorithm>

namespace rl
{
constexpr float kPi     = 3.14159265358979f;
constexpr float kTwoPi  = 6.28318530717959f;
constexpr float kHalfPi = 1.57079632679490f;

enum Wave { WaveSaw = 0, WaveSquare, WaveTriangle, WaveChipPulse, WaveChipSaw, WaveBell, NumWaves };

// Smooths the step at a waveform discontinuity.
inline float polyBlep(float t, float dt)
{
    if (t < dt)
    {
        t /= dt;
        return t + t - t * t - 1.0f;
    }
    if (t > 1.0f - dt)
    {
        t = (t - 1.0f) / dt;
        return t * t + t + t + 1.0f;
    }
    return 0.0f;
}

inline float wrap01(float x)
{
    x -= std::floor(x);
    return x;
}

// A pulse wave with adjustable width (pw = 0.5 is a square).
inline float pulseWave(float phase, float dt, float pw)
{
    float v = phase < pw ? 1.0f : -1.0f;
    v += polyBlep(phase, dt);
    v -= polyBlep(wrap01(phase + (1.0f - pw)), dt);
    return v;
}

inline float renderWave(int wave, float phase, float dt, float pw)
{
    switch (wave)
    {
        default:
        case WaveSaw:
            return 2.0f * phase - 1.0f - polyBlep(phase, dt);

        case WaveSquare:
            return pulseWave(phase, dt, pw);

        case WaveTriangle:
            return 4.0f * std::abs(phase - 0.5f) - 1.0f;

        case WaveChipPulse:
        {
            // NES-style pulse: only three fixed widths (12.5%, 25%, 50%).
            // The PWM knob picks which one.
            const float duty = pw < 0.34f ? 0.125f : (pw < 0.67f ? 0.25f : 0.5f);
            return pulseWave(phase, dt, duty);
        }

        case WaveChipSaw:
        {
            // 8-bit style staircase saw: 16 steps per cycle.
            const float steps = 16.0f;
            float v = std::floor(phase * steps) / (steps - 1.0f) * 2.0f - 1.0f;
            if (dt * steps < 0.5f) // smooth the steps a little when the pitch is not too high
            {
                const float p16 = wrap01(phase * steps);
                v += (1.0f / (steps - 1.0f)) * polyBlep(p16, dt * steps);
                v -= (steps / (steps - 1.0f)) * polyBlep(phase, dt);
            }
            return v;
        }

        case WaveBell:
        {
            // Two-operator FM with an inharmonic ratio (3.5) = bell / "alien" tone.
            const float mod = std::sin(kTwoPi * 3.5f * phase);
            return std::sin(kTwoPi * phase + 1.6f * mod) * 0.9f;
        }
    }
}
} // namespace rl
