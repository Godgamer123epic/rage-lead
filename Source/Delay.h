// ============================================================================
//  Delay.h  -  tempo-synced ping-pong delay with a filtered feedback path
// ============================================================================
#pragma once
#include <vector>
#include <cmath>
#include "Filter.h"

namespace rl
{
class PingPongDelay
{
public:
    void prepare(double sr)
    {
        sampleRate = sr;
        size = (size_t) (sr * 5.0);
        bufL.assign(size, 0.0f);
        bufR.assign(size, 0.0f);
        reset();
    }

    void reset()
    {
        std::fill(bufL.begin(), bufL.end(), 0.0f);
        std::fill(bufR.begin(), bufR.end(), 0.0f);
        writePos = 0;
        mixS = 0.0f;
        delaySmoothed = -1.0f;
        fbFilterL.reset(); fbFilterR.reset();
        hpL = hpR = hpXL = hpXR = 0.0f;
    }

    // delaySeconds: time of one echo.  feedback 0..0.9.  tone 0..1 (dark..bright)
    void process(float* L, float* R, int n, bool on, double delaySeconds, float feedback, float tone, float mix)
    {
        const float target = on ? mix : 0.0f;
        const float targetDelay = (float) std::min(std::max(delaySeconds * sampleRate, 32.0), (double) size - 4.0);
        if (delaySmoothed < 0.0f) delaySmoothed = targetDelay;

        const float toneHz = 600.0f * std::pow(16.0f, std::min(std::max(tone, 0.0f), 1.0f)); // 600 Hz .. 9.6 kHz
        const SvfCoeffs tc = makeSvfCoeffs(toneHz, 0.05f, sampleRate);
        feedback = std::min(std::max(feedback, 0.0f), 0.92f);

        for (int i = 0; i < n; ++i)
        {
            mixS += (target - mixS) * 0.002f;
            delaySmoothed += (targetDelay - delaySmoothed) * 0.0005f;

            const float dl = readDelay(bufL, delaySmoothed);
            const float dr = readDelay(bufR, delaySmoothed);

            // feedback path: low-pass (tone) + gentle high-pass (keeps it from getting muddy)
            float fl = fbFilterL.process(dr, tc, 0, 0.0f);
            float fr = fbFilterR.process(dl, tc, 0, 0.0f);
            const float hl = fl - hpXL + 0.985f * hpL; hpXL = fl; hpL = hl;
            const float hr = fr - hpXR + 0.985f * hpR; hpXR = fr; hpR = hr;

            const float in = 0.5f * (L[i] + R[i]);
            bufL[writePos] = in + fastTanh(hl * feedback);
            bufR[writePos] = fastTanh(hr * feedback);
            if (++writePos >= size) writePos = 0;

            L[i] += dl * mixS;
            R[i] += dr * mixS;
        }
    }

private:
    float readDelay(const std::vector<float>& buf, float d) const
    {
        float rp = (float) writePos - d;
        while (rp < 0.0f) rp += (float) size;
        const size_t i0 = (size_t) rp % size;
        const size_t i1 = (i0 + 1) % size;
        const float fr = rp - std::floor(rp);
        return buf[i0] + (buf[i1] - buf[i0]) * fr;
    }

    double sampleRate = 44100.0;
    size_t size = 1, writePos = 0;
    std::vector<float> bufL, bufR;
    float mixS = 0.0f, delaySmoothed = -1.0f;
    SvfChannel fbFilterL, fbFilterR;
    float hpL = 0, hpR = 0, hpXL = 0, hpXR = 0;
};
} // namespace rl
