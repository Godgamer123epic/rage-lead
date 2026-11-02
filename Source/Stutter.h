// ============================================================================
//  Stutter.h  -  glitch effects: tempo-synced Stutter and Tape Stop
// ============================================================================
#pragma once
#include <vector>
#include <cmath>
#include <algorithm>

namespace rl
{
//------------------------------------------------------------------------------
//  Stutter: the beat is cut into slices.  Every 4th slice plays normally while
//  it is recorded; the next 3 slices repeat that recording = classic "glitch".
//------------------------------------------------------------------------------
class Stutter
{
public:
    void prepare(double sr)
    {
        sampleRate = sr;
        maxSlice = (size_t) (sr * 3.0);
        capL.assign(maxSlice, 0.0f);
        capR.assign(maxSlice, 0.0f);
        reset();
    }

    void reset()
    {
        std::fill(capL.begin(), capL.end(), 0.0f);
        std::fill(capR.begin(), capR.end(), 0.0f);
        mixS = 0.0f;
        freePos = 0.0;
        recorded = 0;
    }

    // beatsPerSlice: e.g. 0.25 = 1/16 note.  ppq < 0 means "host gives no position"
    void process(float* L, float* R, int n, bool on, float mix, double beatsPerSlice,
                 double bpm, double ppq, bool hostPlaying)
    {
        const float target = on ? mix : 0.0f;
        if (target < 0.0005f && mixS < 0.0005f)
        {
            mixS = 0.0f;
            return;
        }

        const double sliceLen = std::max(64.0, std::min(beatsPerSlice * 60.0 / std::max(bpm, 20.0) * sampleRate,
                                                        (double) maxSlice - 1.0));
        // where are we on the slice grid?
        double posInGrid; // measured in samples from a bar-aligned start
        if (hostPlaying && ppq >= 0.0)
            posInGrid = ppq * 60.0 / std::max(bpm, 20.0) * sampleRate;
        else
            posInGrid = freePos;

        const float fadeLen = 64.0f;

        for (int i = 0; i < n; ++i)
        {
            const double gridPos = posInGrid + (double) i;
            const double sliceIdx = std::floor(gridPos / sliceLen);
            const double inSlice = gridPos - sliceIdx * sliceLen;
            const int cyclePos = (int) std::fmod(sliceIdx, 4.0);

            mixS += (target - mixS) * 0.002f;
            const float dl = L[i], dr = R[i];

            if (cyclePos == 0)
            {
                // record slice (live signal passes through untouched)
                const size_t idx = (size_t) inSlice;
                if (idx < maxSlice)
                {
                    capL[idx] = dl;
                    capR[idx] = dr;
                    recorded = idx + 1;
                }
            }
            else if (recorded > 8)
            {
                // repeat slice
                const size_t idx = std::min((size_t) inSlice, recorded - 1);
                const float fin  = std::min(1.0f, (float) inSlice / fadeLen);
                const float fout = std::min(1.0f, (float) (sliceLen - inSlice) / fadeLen);
                const float w = std::min(fin, fout) * mixS;
                L[i] = dl + (capL[idx] - dl) * w;
                R[i] = dr + (capR[idx] - dr) * w;
            }
        }
        freePos += (double) n;
    }

private:
    double sampleRate = 44100.0;
    size_t maxSlice = 1, recorded = 0;
    std::vector<float> capL, capR;
    float mixS = 0.0f;
    double freePos = 0.0;
};

//------------------------------------------------------------------------------
//  Tape stop: when triggered, the sound slows down and drops in pitch until it
//  stops, like pressing stop on a tape machine.  Release = back to normal.
//------------------------------------------------------------------------------
class TapeStop
{
public:
    void prepare(double sr)
    {
        sampleRate = sr;
        size = (size_t) (sr * 4.0);
        bufL.assign(size, 0.0f);
        bufR.assign(size, 0.0f);
        reset();
    }

    void reset()
    {
        std::fill(bufL.begin(), bufL.end(), 0.0f);
        std::fill(bufR.begin(), bufR.end(), 0.0f);
        writePos = 0;
        readPos = 0.0;
        speed = 1.0f;
        wetMix = 0.0f;
        engaged = false;
    }

    void process(float* L, float* R, int n, bool trigger, float timeMs)
    {
        if (! trigger && ! engaged && wetMix < 0.0005f)
        {
            // idle: still record so that the buffer is ready
            for (int i = 0; i < n; ++i)
            {
                bufL[writePos] = L[i];
                bufR[writePos] = R[i];
                if (++writePos >= size) writePos = 0;
            }
            return;
        }

        const float slowPerSample = 1.0f / std::max(1.0f, timeMs * 0.001f * (float) sampleRate);
        const float attackCoef  = 1.0f - std::exp(-1.0f / (0.003f * (float) sampleRate));
        const float releaseCoef = 1.0f - std::exp(-1.0f / (0.030f * (float) sampleRate));

        if (trigger && ! engaged)
        {
            engaged = true;
            speed = 1.0f;
            readPos = (double) writePos;      // start exactly at "now"
        }
        if (! trigger && engaged)
            engaged = false;                  // crossfade back to live sound

        for (int i = 0; i < n; ++i)
        {
            bufL[writePos] = L[i];
            bufR[writePos] = R[i];
            if (++writePos >= size) writePos = 0;

            if (engaged)
            {
                speed = std::max(0.0f, speed - slowPerSample);
                readPos += (double) speed;
                if (readPos >= (double) size) readPos -= (double) size;
            }

            // linear interpolated read
            const size_t i0 = (size_t) readPos;
            const size_t i1 = (i0 + 1) % size;
            const float fr = (float) (readPos - (double) i0);
            const float wl = (bufL[i0] + (bufL[i1] - bufL[i0]) * fr) * speed;
            const float wr = (bufR[i0] + (bufR[i1] - bufR[i0]) * fr) * speed;

            wetMix += ((engaged ? 1.0f : 0.0f) - wetMix) * (engaged ? attackCoef : releaseCoef);
            L[i] += (wl - L[i]) * wetMix;
            R[i] += (wr - R[i]) * wetMix;
        }
        if (! engaged && wetMix < 0.0005f)
            wetMix = 0.0f;
    }

private:
    double sampleRate = 44100.0;
    size_t size = 1, writePos = 0;
    double readPos = 0.0;
    float speed = 1.0f, wetMix = 0.0f;
    bool engaged = false;
    std::vector<float> bufL, bufR;
};
} // namespace rl
