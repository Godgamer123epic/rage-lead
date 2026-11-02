// ============================================================================
//  Lfo.h  -  a low frequency oscillator used for movement (wobble, vibrato...)
// ============================================================================
#pragma once
#include <juce_core/juce_core.h>
#include "Oscillator.h"

namespace rl
{
class Lfo
{
public:
    double phase = 0.0;

    // shape: 0 sine, 1 saw, 2 square, 3 random (sample & hold)
    // inc = how much the phase moves each sample.  Returns a value from -1 to +1.
    float next(double inc, int shape)
    {
        phase += inc;
        if (phase >= 1.0)
        {
            phase -= std::floor(phase);
            held = rng.nextFloat() * 2.0f - 1.0f; // new random value each cycle
        }
        const float p = (float) phase;
        switch (shape)
        {
            case 0:  return std::sin(kTwoPi * p);
            case 1:  return 1.0f - 2.0f * p;
            case 2:  return p < 0.5f ? 1.0f : -1.0f;
            default: return held;
        }
    }

private:
    float held = 0.0f;
    juce::Random rng { 12345 };
};
} // namespace rl
