// ============================================================================
//  Voice.h  -  one synth voice, and the SynthEngine that manages 8 of them.
//
//  A voice = 2 oscillators (each with up to 7 unison copies) + a sub oscillator
//            -> resonant filter -> amplifier (volume envelope).
//  The engine handles MIDI (notes, pitch bend, mod wheel, sustain pedal),
//  poly / mono / legato modes, glide and the two LFOs.
// ============================================================================
#pragma once
#include <juce_audio_basics/juce_audio_basics.h>
#include <array>
#include "Oscillator.h"
#include "Filter.h"
#include "Lfo.h"
#include "Parameters.h"

namespace rl
{
//------------------------------------------------------------------------------
// All the synth settings for the current moment (already includes macro effects)
//------------------------------------------------------------------------------
struct SynthParams
{
    int   o1wave = 0, o2wave = 0;
    float o1level = 0.8f, o1pwm = 0.5f, o2level = 0.6f, o2detune = 0.0f, o2semi = 0.0f, subLevel = 0.0f;
    int   uniVoices = 1;
    float uniSpread = 0.0f;
    int   voiceMode = 0;          // 0 poly, 1 mono, 2 legato
    float glideMs = 0.0f;

    int   fType = 0;
    float cutoff = 4000.0f, res = 0.2f, fDrive = 0.0f, keytrack = 0.0f, fEnvAmt = 0.0f;
    float aA = 2, aD = 300, aS = 0.8f, aR = 100;      // amp envelope (ms, ms, level, ms)
    float fA = 1, fD = 250, fS = 0.2f, fR = 150;      // filter envelope

    float lfoRate[2]  = { 5.0f, 1.0f };
    int   lfoSync[2]  = { 0, 0 };
    int   lfoShape[2] = { 0, 1 };

    // mod[source][destination]; source 0 = LFO1, 1 = LFO2, 2 = filter envelope
    // destination 0 = pitch, 1 = cutoff, 2 = PWM, 3 = distortion, 4 = unison detune
    float mod[3][5] = {};

    double bpm = 120.0;
    double ppq = 0.0;
    bool   hostPlaying = false;
};

// bipolar amount -> semitones (squared curve so small amounts give gentle vibrato)
inline float modToSemis(float a) { return (a < 0.0f ? -a * a : a * a) * 12.0f; }

//==============================================================================
class Voice
{
public:
    void prepare(double sr)
    {
        sampleRate = sr;
        ampEnv.setSampleRate(sr);
        fltEnv.setSampleRate(sr);
        ampEnv.reset();
        fltEnv.reset();
        filtL.reset();
        filtR.reset();
        active = false;
    }

    bool isActive() const { return active; }
    bool isGateOn() const { return gate; }
    float getFilterEnvValue() const { return fltEnvLast; }

    // Start (or restart) this voice on a note.
    //   startNote: pitch to glide from.   freshPhases: randomise oscillator phases
    void start(int note, float velocity, float startNote, bool freshPhases, bool retriggerEnvelopes)
    {
        noteNumber = note;
        vel = velocity;
        noteTarget = (float) note;
        noteCur = startNote;
        if (freshPhases)
        {
            for (int i = 0; i < 7; ++i)
            {
                ph1[i] = randomPhase();
                ph2[i] = randomPhase();
            }
            subPhase = 0.0f;
        }
        if (retriggerEnvelopes || ! active)
        {
            ampEnv.noteOn();
            fltEnv.noteOn();
        }
        active = true;
        gate = true;
        keyDown = true;
    }

    // Change pitch only (used for legato playing)
    void retarget(int note)
    {
        noteNumber = note;
        noteTarget = (float) note;
        keyDown = true;
    }

    void release()
    {
        if (gate)
        {
            ampEnv.noteOff();
            fltEnv.noteOff();
        }
        gate = false;
    }

    void kill()
    {
        ampEnv.reset();
        fltEnv.reset();
        filtL.reset();
        filtR.reset();
        active = false;
        gate = false;
        keyDown = false;
    }

    //--------------------------------------------------------------------------
    // Render n samples and ADD them to outL / outR.
    // lfo1 / lfo2 hold one LFO value per sample.
    void render(float* outL, float* outR, int n, const SynthParams& p,
                const float* lfo1, const float* lfo2, float bendSemis, float wheel)
    {
        if (! active)
            return;

        const float sr = (float) sampleRate;

        //---- values that only need updating once per little chunk ---------------
        const int nU = std::min(std::max(p.uniVoices, 1), 7);

        const float lfoA = lfo1[0], lfoB = lfo2[0];
        const float envNow = fltEnvLast;

        const float pwmMod = (p.mod[0][2] * lfoA + p.mod[1][2] * lfoB + p.mod[2][2] * envNow) * 0.45f;
        const float uniMod = (p.mod[0][4] * lfoA + p.mod[1][4] * lfoB + p.mod[2][4] * envNow) * 0.5f;
        const float pw = std::min(std::max(p.o1pwm + pwmMod, 0.05f), 0.95f);

        const float spread = std::min(std::max(p.uniSpread + uniMod, 0.0f), 1.0f);
        const float spreadCents = spread * 70.0f;
        const float stereoW = std::min(1.0f, 0.25f + spread);

        float detMul[7], gainL[7], gainR[7];
        for (int i = 0; i < nU; ++i)
        {
            const float pos = nU > 1 ? 2.0f * (float) i / (float) (nU - 1) - 1.0f : 0.0f;
            detMul[i] = std::exp2(pos * spreadCents / 1200.0f);
            const float ang = (pos * stereoW * 0.5f + 0.5f) * kHalfPi;
            gainL[i] = std::cos(ang);
            gainR[i] = std::sin(ang);
        }
        const float uniNorm = 1.0f / std::sqrt((float) nU);
        const float uniCompensate = nU > 1 ? 1.4142f : 1.0f; // centre voice is louder than pan-law average

        // glide speed
        float glideCoef = 1.0f;
        if (p.glideMs > 0.5f)
            glideCoef = 1.0f - std::exp(-1.0f / ((p.glideMs * 0.001f / 3.0f) * sr));

        // envelopes (enforce tiny minimum times so notes never click)
        juce::ADSR::Parameters ap, fp;
        ap.attack = std::max(p.aA, 0.5f) * 0.001f;  ap.decay = std::max(p.aD, 1.0f) * 0.001f;
        ap.sustain = p.aS;                          ap.release = std::max(p.aR, 3.0f) * 0.001f;
        fp.attack = std::max(p.fA, 0.5f) * 0.001f;  fp.decay = std::max(p.fD, 1.0f) * 0.001f;
        fp.sustain = p.fS;                          fp.release = std::max(p.fR, 3.0f) * 0.001f;
        ampEnv.setParameters(ap);
        fltEnv.setParameters(fp);

        const float pitchA = modToSemis(p.mod[0][0]);
        const float pitchB = modToSemis(p.mod[1][0]);
        const float pitchE = modToSemis(p.mod[2][0]);

        const float osc2Semis = p.o2semi + p.o2detune / 100.0f;
        const float velGain = 0.3f + 0.7f * vel;

        for (int i = 0; i < n; ++i)
        {
            const float la = lfo1[i], lb = lfo2[i];
            const float fe = fltEnv.getNextSample();
            const float ae = ampEnv.getNextSample();
            fltEnvLast = fe;

            // ---- pitch ---------------------------------------------------------
            noteCur += (noteTarget - noteCur) * glideCoef;
            const float semis = noteCur - 69.0f + bendSemis + pitchA * la + pitchB * lb + pitchE * fe;
            const float f1 = 440.0f * std::exp2(semis / 12.0f);
            const float f2 = f1 * std::exp2(osc2Semis / 12.0f);
            const float dt1 = std::min(f1 / sr, 0.45f);
            const float dt2 = std::min(f2 / sr, 0.45f);

            // ---- oscillators (with unison) --------------------------------------
            float sumL = 0.0f, sumR = 0.0f;
            for (int u = 0; u < nU; ++u)
            {
                const float d1 = std::min(dt1 * detMul[u], 0.45f);
                const float d2 = std::min(dt2 * detMul[u], 0.45f);
                const float w1 = renderWave(p.o1wave, ph1[u], d1, pw) * p.o1level;
                const float w2 = renderWave(p.o2wave, ph2[u], d2, pw) * p.o2level;
                const float w = w1 + w2;
                sumL += w * gainL[u];
                sumR += w * gainR[u];
                ph1[u] += d1; if (ph1[u] >= 1.0f) ph1[u] -= 1.0f;
                ph2[u] += d2; if (ph2[u] >= 1.0f) ph2[u] -= 1.0f;
            }
            sumL *= uniNorm * uniCompensate * 0.5f;
            sumR *= uniNorm * uniCompensate * 0.5f;

            // sub oscillator: a pure sine one octave down, centred
            if (p.subLevel > 0.001f)
            {
                const float sub = std::sin(kTwoPi * subPhase) * p.subLevel * 0.7f;
                sumL += sub;
                sumR += sub;
            }
            subPhase += dt1 * 0.5f;
            if (subPhase >= 1.0f) subPhase -= 1.0f;

            // ---- filter ----------------------------------------------------------
            const float cutOct = p.fEnvAmt * fe * 5.0f
                               + (p.mod[0][1] * la + p.mod[1][1] * lb + p.mod[2][1] * fe) * 4.0f
                               + wheel * 3.0f
                               + p.keytrack * (noteCur - 60.0f) / 12.0f;
            const float fc = p.cutoff * std::exp2(std::min(std::max(cutOct, -10.0f), 10.0f));
            const SvfCoeffs coeffs = makeSvfCoeffs(fc, p.res, sampleRate);
            const float fl = filtL.process(sumL, coeffs, p.fType, p.fDrive);
            const float fr = filtR.process(sumR, coeffs, p.fType, p.fDrive);

            // ---- amp -------------------------------------------------------------
            const float g = ae * velGain;
            outL[i] += fl * g;
            outR[i] += fr * g;
        }

        if (! ampEnv.isActive())
        {
            active = false;
            gate = false;
            keyDown = false;
            filtL.reset();
            filtR.reset();
        }
    }

    int   noteNumber = -1;
    bool  keyDown = false;   // is the physical key still held?
    unsigned long age = 0;   // used to find the oldest voice when stealing
    float noteCur = 60.0f;   // current (gliding) pitch in MIDI note units

private:
    float randomPhase()
    {
        rngState ^= rngState << 13;
        rngState ^= rngState >> 17;
        rngState ^= rngState << 5;
        return (float) (rngState & 0xFFFFFF) / 16777216.0f;
    }

    double sampleRate = 44100.0;
    bool active = false, gate = false;
    float vel = 1.0f;
    float noteTarget = 60.0f;
    float ph1[7] = {}, ph2[7] = {}, subPhase = 0.0f;
    float fltEnvLast = 0.0f;
    juce::ADSR ampEnv, fltEnv;
    SvfChannel filtL, filtR;
    uint32_t rngState = 2463534242u;
};

//==============================================================================
class SynthEngine
{
public:
    static constexpr int kVoices = 8;
    static constexpr int kChunk  = 32;

    void prepare(double sr)
    {
        sampleRate = sr;
        for (auto& v : voices) v.prepare(sr);
        smoothCoef = 1.0f - std::exp(-(float) kChunk / (0.006f * (float) sr));
        bendCoef = 1.0f - std::exp(-(float) kChunk / (0.004f * (float) sr));
        needInit = true;
        noteCount = 0;
        lastNote = -1.0f;
        pedal = false;
        bend = bendTarget = wheel = wheelTarget = 0.0f;
    }

    // Called once at the start of each audio block with the newest settings.
    void setTarget(const SynthParams& t)
    {
        target = t;
        if (needInit)
        {
            cur = t;
            needInit = false;
        }
        // Lock synced LFOs to the host's beat position
        for (int k = 0; k < 2; ++k)
            if (t.lfoSync[k] > 0 && t.hostPlaying)
            {
                const double beats = lfoSyncBeats[std::min(t.lfoSync[k], 8)];
                lfo[k].phase = std::fmod(t.ppq / beats, 1.0);
                if (lfo[k].phase < 0.0) lfo[k].phase += 1.0;
            }
        // if we left poly mode, make sure extra voices fade out
        if (t.voiceMode != 0)
            for (int i = 1; i < kVoices; ++i)
                if (voices[i].isGateOn()) { voices[i].release(); }
    }

    float getDistMod() const { return distMod; }

    //---------------------------------------------------------------- MIDI ----
    void noteOn(int note, float velocity)
    {
        // remember held notes (for mono mode)
        if (noteCount < kMaxStack)
        {
            stackNote[noteCount] = note;
            stackVel[noteCount] = velocity;
            ++noteCount;
        }

        if (target.voiceMode == 0)
        {
            Voice* v = findVoiceFor(note);
            const float from = lastNote >= 0.0f ? lastNote : (float) note;
            const bool fresh = ! v->isActive();
            v->start(note, velocity, from, fresh, true);
            v->age = ++ageCounter;
        }
        else
        {
            Voice& v = voices[0];
            const bool playing = v.isActive() && v.isGateOn();
            if (playing && target.voiceMode == 2)
            {
                v.retarget(note);                                   // legato: glide, no retrigger
            }
            else
            {
                const float from = v.isActive() ? v.noteCur : (lastNote >= 0.0f ? lastNote : (float) note);
                v.start(note, velocity, from, ! v.isActive(), true);
            }
            v.age = ++ageCounter;
        }
        lastNote = (float) note;
    }

    void noteOff(int note)
    {
        // take it out of the held-note list
        int found = -1;
        for (int i = 0; i < noteCount; ++i)
            if (stackNote[i] == note) found = i;
        if (found >= 0)
        {
            for (int i = found; i < noteCount - 1; ++i)
            {
                stackNote[i] = stackNote[i + 1];
                stackVel[i] = stackVel[i + 1];
            }
            --noteCount;
        }

        if (target.voiceMode == 0)
        {
            for (auto& v : voices)
                if (v.isActive() && v.isGateOn() && v.keyDown && v.noteNumber == note)
                {
                    v.keyDown = false;
                    if (! pedal) v.release();
                }
        }
        else
        {
            Voice& v = voices[0];
            if (! v.isActive() || v.noteNumber != note)
                return;
            if (noteCount > 0)
            {
                const int prev = stackNote[noteCount - 1];     // fall back to the previous held key
                if (target.voiceMode == 2) v.retarget(prev);
                else v.start(prev, stackVel[noteCount - 1], v.noteCur, false, true);
                lastNote = (float) prev;
            }
            else
            {
                v.keyDown = false;
                if (! pedal) v.release();
            }
        }
    }

    void controller(int cc, int value)
    {
        if (cc == 1)
            wheelTarget = (float) value / 127.0f;
        else if (cc == 64)
        {
            const bool down = value >= 64;
            if (pedal && ! down)
                for (auto& v : voices)
                    if (v.isActive() && v.isGateOn() && ! v.keyDown)
                        v.release();
            pedal = down;
        }
        else if (cc == 120 || cc == 123)
            allNotesOff();
    }

    void pitchWheel(int value14)
    {
        bendTarget = ((float) value14 - 8192.0f) / 8192.0f * 2.0f; // +/- 2 semitones
    }

    void allNotesOff()
    {
        noteCount = 0;
        for (auto& v : voices)
            if (v.isActive()) v.release();
    }

    //-------------------------------------------------------------- render ----
    void render(float* L, float* R, int n)
    {
        int pos = 0;
        while (pos < n)
        {
            const int m = std::min(kChunk, n - pos);
            updateSmoothedParams();
            bend += (bendTarget - bend) * bendCoef;
            wheel += (wheelTarget - wheel) * bendCoef;

            // LFOs for this chunk
            for (int k = 0; k < 2; ++k)
            {
                double inc;
                if (cur.lfoSync[k] == 0)
                    inc = (double) cur.lfoRate[k] / sampleRate;
                else
                {
                    const double beats = lfoSyncBeats[std::min(cur.lfoSync[k], 8)];
                    inc = (cur.bpm / 60.0) / (beats * sampleRate);
                }
                for (int i = 0; i < m; ++i)
                    lfoBuf[k][i] = lfo[k].next(inc, cur.lfoShape[k]);
            }

            float envMax = 0.0f;
            for (auto& v : voices)
                if (v.isActive())
                {
                    v.render(L + pos, R + pos, m, cur, lfoBuf[0], lfoBuf[1], bend, wheel);
                    envMax = std::max(envMax, v.getFilterEnvValue());
                }

            distMod = cur.mod[0][3] * lfoBuf[0][0] + cur.mod[1][3] * lfoBuf[1][0] + cur.mod[2][3] * envMax
                      + wheel * 0.3f;
            pos += m;
        }
    }

private:
    static constexpr int kMaxStack = 32;

    Voice* findVoiceFor(int note)
    {
        // 1) the same note already playing -> reuse it (retrigger)
        for (auto& v : voices)
            if (v.isActive() && v.noteNumber == note && v.keyDown) return &v;
        // 2) a free voice
        for (auto& v : voices)
            if (! v.isActive()) return &v;
        // 3) steal: oldest voice that is already releasing
        Voice* best = nullptr;
        for (auto& v : voices)
            if (! v.isGateOn() && (best == nullptr || v.age < best->age)) best = &v;
        if (best != nullptr) return best;
        // 4) steal: the oldest voice overall
        best = &voices[0];
        for (auto& v : voices)
            if (v.age < best->age) best = &v;
        return best;
    }

    // Moves the "current" settings a small step towards the target = no zipper noise.
    void updateSmoothedParams()
    {
        const float a = smoothCoef;
        auto sm = [a](float& c, float t) { c += (t - c) * a; };

        SynthParams t = target;
        // values that are smoothed
        sm(cur.o1level, t.o1level);   sm(cur.o1pwm, t.o1pwm);
        sm(cur.o2level, t.o2level);   sm(cur.o2detune, t.o2detune);
        sm(cur.o2semi, t.o2semi);     sm(cur.subLevel, t.subLevel);
        sm(cur.uniSpread, t.uniSpread);
        sm(cur.res, t.res);           sm(cur.fDrive, t.fDrive);
        sm(cur.keytrack, t.keytrack); sm(cur.fEnvAmt, t.fEnvAmt);
        // cutoff is smoothed in the log (musical) domain
        {
            float lc = std::log2(cur.cutoff), lt = std::log2(t.cutoff);
            lc += (lt - lc) * a;
            cur.cutoff = std::exp2(lc);
        }
        for (int s = 0; s < 3; ++s)
            for (int d = 0; d < 5; ++d)
                sm(cur.mod[s][d], t.mod[s][d]);

        // everything else is copied straight across
        cur.o1wave = t.o1wave;  cur.o2wave = t.o2wave;
        cur.uniVoices = t.uniVoices;  cur.voiceMode = t.voiceMode;  cur.glideMs = t.glideMs;
        cur.fType = t.fType;
        cur.aA = t.aA; cur.aD = t.aD; cur.aS = t.aS; cur.aR = t.aR;
        cur.fA = t.fA; cur.fD = t.fD; cur.fS = t.fS; cur.fR = t.fR;
        for (int k = 0; k < 2; ++k)
        {
            cur.lfoRate[k] = t.lfoRate[k]; cur.lfoSync[k] = t.lfoSync[k]; cur.lfoShape[k] = t.lfoShape[k];
        }
        cur.bpm = t.bpm; cur.ppq = t.ppq; cur.hostPlaying = t.hostPlaying;
    }

    double sampleRate = 44100.0;
    std::array<Voice, kVoices> voices;
    SynthParams target, cur;
    bool needInit = true;
    float smoothCoef = 0.2f, bendCoef = 0.3f;
    Lfo lfo[2];
    float lfoBuf[2][kChunk] = {};
    float distMod = 0.0f;

    float bend = 0.0f, bendTarget = 0.0f, wheel = 0.0f, wheelTarget = 0.0f;
    bool pedal = false;
    float lastNote = -1.0f;
    unsigned long ageCounter = 0;

    int stackNote[kMaxStack] = {};
    float stackVel[kMaxStack] = {};
    int noteCount = 0;
};
} // namespace rl
