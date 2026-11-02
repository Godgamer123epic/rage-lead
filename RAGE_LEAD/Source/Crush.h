// ============================================================================
//  Crush.h  -  bit-depth + sample-rate reducer (intentional lo-fi grit)
// ============================================================================
#pragma once
#include <cmath>
#include <algorithm>

namespace rl
{
class Crush
{
public:
    void prepare(double) { reset(); }
    void reset() { heldL = heldR = 0.0f; counter = 0.0f; mixS = 0.0f; }

    // bits: 3..16   rateFactor: 1 = no reduction, 10 = every 10th sample is kept
    void process(float* L, float* R, int n, float bits, float rateFactor, float mix)
    {
        if (mix < 0.0001f && mixS < 0.0001f)
        {
            mixS = 0.0f;
            return;
        }
        const float levels = std::exp2(std::max(bits, 2.0f) - 1.0f);
        rateFactor = std::max(rateFactor, 1.0f);

        for (int i = 0; i < n; ++i)
        {
            mixS += (mix - mixS) * 0.002f;
            counter += 1.0f;
            if (counter >= rateFactor)
            {
                counter -= rateFactor;
                heldL = std::round(L[i] * levels) / levels;
                heldR = std::round(R[i] * levels) / levels;
            }
            L[i] += (heldL - L[i]) * mixS;
            R[i] += (heldR - R[i]) * mixS;
        }
    }

private:
    float heldL = 0.0f, heldR = 0.0f, counter = 0.0f, mixS = 0.0f;
};
} // namespace rl
