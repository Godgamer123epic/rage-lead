// ============================================================================
//  Parameters.h  -  every knob, button and menu in RAGE LEAD is defined here.
//
//  Each parameter has:  an ID (used in code and saved in presets),
//                       a display name (what FL Studio shows for automation),
//                       a range, a default value and a "unit" for the readout.
//  They all live in the AudioProcessorValueTreeState, which means they are
//  automatable in FL Studio and saved with your project.
// ============================================================================
#pragma once
#include <juce_audio_processors/juce_audio_processors.h>

namespace rl
{
//------------------------------------------------------------------------------
// The master list of parameter IDs.  (An "X-macro": one list, used twice below
// to build both an enum of indices and an array of text IDs.)
//------------------------------------------------------------------------------
#define RL_PARAM_LIST(X) \
    X(macroRage) X(macroWidth) X(macroSpace) X(macroBright) \
    X(o1wave) X(o1level) X(o1pwm) X(o2wave) X(o2level) X(o2detune) X(o2semi) X(subLevel) \
    X(uniVoices) X(uniSpread) X(voiceMode) X(glide) \
    X(fType) X(cutoff) X(res) X(fDrive) X(keytrack) X(fEnvAmt) \
    X(aA) X(aD) X(aS) X(aR) X(fA) X(fD) X(fS) X(fR) \
    X(l1shape) X(l1sync) X(l1rate) X(l2shape) X(l2sync) X(l2rate) \
    X(l1Pitch) X(l1Cut) X(l1Pwm) X(l1Dist) X(l1Uni) \
    X(l2Pitch) X(l2Cut) X(l2Pwm) X(l2Dist) X(l2Uni) \
    X(ePitch) X(eCut) X(ePwm) X(eDist) X(eUni) \
    X(crushOn) X(crushBits) X(crushRate) X(crushMix) \
    X(distOn) X(distMode) X(distDrive) X(distTone) \
    X(stutOn) X(stutRate) X(stutMix) X(tapeStop) X(tapeTime) \
    X(choOn) X(choMix) X(choRate) \
    X(dlyOn) X(dlySync) X(dlyFb) X(dlyTone) X(dlyMix) \
    X(revOn) X(revSize) X(revDamp) X(revPre) X(revMix) X(revGate) X(revGateTime) \
    X(master) X(limOn)

namespace P
{
    enum Id
    {
#define RL_X(n) n,
        RL_PARAM_LIST(RL_X)
#undef RL_X
        Count
    };
}

inline const char* const paramIds[] = {
#define RL_X(n) #n,
    RL_PARAM_LIST(RL_X)
#undef RL_X
};

//------------------------------------------------------------------------------
// Menu contents
//------------------------------------------------------------------------------
inline const juce::StringArray& waveNames()
{
    static const juce::StringArray s { "Saw", "Square", "Triangle", "Chip Pulse", "Chip Saw", "Bell FM" };
    return s;
}
inline const juce::StringArray& filterTypeNames()
{
    static const juce::StringArray s { "Low-pass", "High-pass", "Band-pass" };
    return s;
}
inline const juce::StringArray& voiceModeNames()
{
    static const juce::StringArray s { "Poly", "Mono", "Legato" };
    return s;
}
inline const juce::StringArray& lfoShapeNames()
{
    static const juce::StringArray s { "Sine", "Saw", "Square", "Random" };
    return s;
}
inline const juce::StringArray& lfoSyncNames()
{
    static const juce::StringArray s { "Free (Hz)", "1/1", "1/2", "1/4", "1/8", "1/16", "1/8T", "1/16T", "1/32" };
    return s;
}
// length of each LFO sync choice in beats (quarter notes); index 0 = free running
inline constexpr double lfoSyncBeats[] = { 0.0, 4.0, 2.0, 1.0, 0.5, 0.25, 1.0 / 3.0, 1.0 / 6.0, 0.125 };

inline const juce::StringArray& distModeNames()
{
    static const juce::StringArray s { "Hard Clip", "Tube", "Wavefold", "Bit-Fold" };
    return s;
}
inline const juce::StringArray& stutRateNames()
{
    static const juce::StringArray s { "1/4", "1/8", "1/8T", "1/16", "1/16T", "1/32", "1/32T" };
    return s;
}
inline constexpr double stutRateBeats[] = { 1.0, 0.5, 1.0 / 3.0, 0.25, 1.0 / 6.0, 0.125, 1.0 / 12.0 };

inline const juce::StringArray& delaySyncNames()
{
    static const juce::StringArray s { "1/16", "1/8T", "1/8", "1/8 dotted", "1/4T", "1/4", "1/4 dotted", "1/2" };
    return s;
}
inline constexpr double delaySyncBeats[] = { 0.25, 1.0 / 3.0, 0.5, 0.75, 2.0 / 3.0, 1.0, 1.5, 2.0 };

//------------------------------------------------------------------------------
// Text shown on the knobs (units)
//------------------------------------------------------------------------------
enum class Unit { Plain, Hz, Ms, Percent, DB, Cents, Semis, Bits, Times };

inline juce::String formatValue(Unit u, float v)
{
    switch (u)
    {
        case Unit::Hz:
            if (v >= 1000.0f) return juce::String(v / 1000.0f, 2) + " kHz";
            if (v >= 100.0f)  return juce::String((int) std::round(v)) + " Hz";
            return juce::String(v, v < 10.0f ? 2 : 1) + " Hz";
        case Unit::Ms:
            if (v >= 1000.0f) return juce::String(v / 1000.0f, 2) + " s";
            if (v >= 100.0f)  return juce::String((int) std::round(v)) + " ms";
            return juce::String(v, 1) + " ms";
        case Unit::Percent: return juce::String((int) std::round(v * 100.0f)) + " %";
        case Unit::DB:      return juce::String(v, 1) + " dB";
        case Unit::Cents:   return juce::String((int) std::round(v)) + " ct";
        case Unit::Semis:   return (v > 0 ? "+" : "") + juce::String((int) std::round(v)) + " st";
        case Unit::Bits:    return juce::String(v, 1) + " bit";
        case Unit::Times:   return juce::String(v, 1) + "x";
        default:            return juce::String(v, 2);
    }
}

inline float parseValue(Unit u, const juce::String& text)
{
    float v = text.retainCharacters("0123456789.-").getFloatValue();
    if (u == Unit::Hz && text.containsIgnoreCase("k"))   v *= 1000.0f;
    if (u == Unit::Ms && text.containsIgnoreCase("s") && ! text.containsIgnoreCase("ms")) v *= 1000.0f;
    if (u == Unit::Percent) v *= 0.01f;
    return v;
}

//------------------------------------------------------------------------------
// Build the full list of parameters
//------------------------------------------------------------------------------
inline juce::AudioProcessorValueTreeState::ParameterLayout createLayout()
{
    using NR = juce::NormalisableRange<float>;
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    auto id = [](P::Id i) { return juce::ParameterID { paramIds[i], 1 }; };

    // continuous knob
    auto F = [&](P::Id i, const char* name, NR range, float def, Unit u)
    {
        auto attrs = juce::AudioParameterFloatAttributes()
                         .withStringFromValueFunction([u](float v, int) { return formatValue(u, v); })
                         .withValueFromStringFunction([u](const juce::String& t) { return parseValue(u, t); });
        layout.add(std::make_unique<juce::AudioParameterFloat>(id(i), name, range, def, attrs));
    };
    // whole-number knob
    auto I = [&](P::Id i, const char* name, int lo, int hi, int def, Unit u)
    {
        auto attrs = juce::AudioParameterIntAttributes()
                         .withStringFromValueFunction([u](int v, int) { return formatValue(u, (float) v); });
        layout.add(std::make_unique<juce::AudioParameterInt>(id(i), name, lo, hi, def, attrs));
    };
    // drop-down menu
    auto C = [&](P::Id i, const char* name, const juce::StringArray& items, int def)
    {
        layout.add(std::make_unique<juce::AudioParameterChoice>(id(i), name, items, def));
    };
    // on/off button
    auto B = [&](P::Id i, const char* name, bool def)
    {
        layout.add(std::make_unique<juce::AudioParameterBool>(id(i), name, def));
    };
    auto skew = [](float lo, float hi, float centre) { NR r(lo, hi); r.setSkewForCentre(centre); return r; };
    const NR unit01(0.0f, 1.0f);

    // ---- Macros -------------------------------------------------------------
    F(P::macroRage,   "Macro RAGE",   unit01, 0.0f, Unit::Percent);
    F(P::macroWidth,  "Macro WIDTH",  unit01, 0.0f, Unit::Percent);
    F(P::macroSpace,  "Macro SPACE",  unit01, 0.0f, Unit::Percent);
    F(P::macroBright, "Macro BRIGHT", unit01, 0.5f, Unit::Percent);

    // ---- Oscillators --------------------------------------------------------
    C(P::o1wave,  "Osc1 Wave",  waveNames(), 0);
    F(P::o1level, "Osc1 Level", unit01, 0.8f, Unit::Percent);
    F(P::o1pwm,   "Osc1 PWM",   NR(0.05f, 0.95f), 0.5f, Unit::Percent);
    C(P::o2wave,  "Osc2 Wave",  waveNames(), 0);
    F(P::o2level, "Osc2 Level", unit01, 0.6f, Unit::Percent);
    F(P::o2detune,"Osc2 Detune",NR(-100.0f, 100.0f), 12.0f, Unit::Cents);
    I(P::o2semi,  "Osc2 Semitones", -24, 24, 0, Unit::Semis);
    F(P::subLevel,"Sub Level",  unit01, 0.0f, Unit::Percent);
    I(P::uniVoices,"Unison Voices", 1, 7, 3, Unit::Plain);
    F(P::uniSpread,"Unison Spread", unit01, 0.35f, Unit::Percent);
    C(P::voiceMode,"Voice Mode", voiceModeNames(), 0);
    F(P::glide,   "Glide",      skew(0.0f, 2000.0f, 200.0f), 0.0f, Unit::Ms);

    // ---- Filter -------------------------------------------------------------
    C(P::fType,   "Filter Type",   filterTypeNames(), 0);
    F(P::cutoff,  "Cutoff",        skew(20.0f, 20000.0f, 1500.0f), 4200.0f, Unit::Hz);
    F(P::res,     "Resonance",     NR(0.0f, 0.95f), 0.2f, Unit::Percent);
    F(P::fDrive,  "Filter Drive",  unit01, 0.1f, Unit::Percent);
    F(P::keytrack,"Key Tracking",  unit01, 0.4f, Unit::Percent);
    F(P::fEnvAmt, "Filter Env Amount", NR(-1.0f, 1.0f), 0.25f, Unit::Percent);

    // ---- Envelopes ----------------------------------------------------------
    F(P::aA, "Amp Attack",  skew(0.5f, 5000.0f, 150.0f),  2.0f,   Unit::Ms);
    F(P::aD, "Amp Decay",   skew(1.0f, 8000.0f, 500.0f),  300.0f, Unit::Ms);
    F(P::aS, "Amp Sustain", unit01, 0.8f, Unit::Percent);
    F(P::aR, "Amp Release", skew(3.0f, 8000.0f, 500.0f),  120.0f, Unit::Ms);
    F(P::fA, "Filter Attack",  skew(0.5f, 5000.0f, 150.0f), 1.0f,   Unit::Ms);
    F(P::fD, "Filter Decay",   skew(1.0f, 8000.0f, 500.0f), 250.0f, Unit::Ms);
    F(P::fS, "Filter Sustain", unit01, 0.2f, Unit::Percent);
    F(P::fR, "Filter Release", skew(3.0f, 8000.0f, 500.0f), 150.0f, Unit::Ms);

    // ---- LFOs ---------------------------------------------------------------
    C(P::l1shape, "LFO1 Shape", lfoShapeNames(), 0);
    C(P::l1sync,  "LFO1 Sync",  lfoSyncNames(), 0);
    F(P::l1rate,  "LFO1 Rate",  skew(0.05f, 30.0f, 3.0f), 5.0f, Unit::Hz);
    C(P::l2shape, "LFO2 Shape", lfoShapeNames(), 1);
    C(P::l2sync,  "LFO2 Sync",  lfoSyncNames(), 3);
    F(P::l2rate,  "LFO2 Rate",  skew(0.05f, 30.0f, 3.0f), 1.0f, Unit::Hz);

    // ---- Mod matrix (source -> destination, bipolar amounts) -----------------
    const NR bi(-1.0f, 1.0f);
    F(P::l1Pitch, "LFO1 > Pitch",      bi, 0.0f, Unit::Percent);
    F(P::l1Cut,   "LFO1 > Cutoff",     bi, 0.0f, Unit::Percent);
    F(P::l1Pwm,   "LFO1 > PWM",        bi, 0.0f, Unit::Percent);
    F(P::l1Dist,  "LFO1 > Distortion", bi, 0.0f, Unit::Percent);
    F(P::l1Uni,   "LFO1 > Unison Detune", bi, 0.0f, Unit::Percent);
    F(P::l2Pitch, "LFO2 > Pitch",      bi, 0.0f, Unit::Percent);
    F(P::l2Cut,   "LFO2 > Cutoff",     bi, 0.0f, Unit::Percent);
    F(P::l2Pwm,   "LFO2 > PWM",        bi, 0.0f, Unit::Percent);
    F(P::l2Dist,  "LFO2 > Distortion", bi, 0.0f, Unit::Percent);
    F(P::l2Uni,   "LFO2 > Unison Detune", bi, 0.0f, Unit::Percent);
    F(P::ePitch,  "FltEnv > Pitch",      bi, 0.0f, Unit::Percent);
    F(P::eCut,    "FltEnv > Cutoff",     bi, 0.0f, Unit::Percent);
    F(P::ePwm,    "FltEnv > PWM",        bi, 0.0f, Unit::Percent);
    F(P::eDist,   "FltEnv > Distortion", bi, 0.0f, Unit::Percent);
    F(P::eUni,    "FltEnv > Unison Detune", bi, 0.0f, Unit::Percent);

    // ---- Rage FX ------------------------------------------------------------
    B(P::crushOn,   "Crush On", true);
    F(P::crushBits, "Crush Bits", NR(3.0f, 16.0f), 16.0f, Unit::Bits);
    F(P::crushRate, "Crush Rate Reduction", skew(1.0f, 40.0f, 6.0f), 1.0f, Unit::Times);
    F(P::crushMix,  "Crush Amount", unit01, 0.0f, Unit::Percent);

    B(P::distOn,    "Distortion On", true);
    C(P::distMode,  "Distortion Mode", distModeNames(), 1);
    F(P::distDrive, "Distortion Drive", unit01, 0.35f, Unit::Percent);
    F(P::distTone,  "Distortion Tone",  unit01, 0.7f, Unit::Percent);

    B(P::stutOn,    "Stutter On", true);
    C(P::stutRate,  "Stutter Rate", stutRateNames(), 3);
    F(P::stutMix,   "Stutter Amount", unit01, 0.0f, Unit::Percent);
    B(P::tapeStop,  "Tape Stop", false);
    F(P::tapeTime,  "Tape Stop Time", skew(100.0f, 2000.0f, 500.0f), 500.0f, Unit::Ms);

    B(P::choOn,     "Chorus On", true);
    F(P::choMix,    "Chorus Amount", unit01, 0.25f, Unit::Percent);
    F(P::choRate,   "Chorus Rate", skew(0.1f, 4.0f, 0.8f), 0.6f, Unit::Hz);

    B(P::dlyOn,     "Delay On", true);
    C(P::dlySync,   "Delay Time", delaySyncNames(), 3);
    F(P::dlyFb,     "Delay Feedback", NR(0.0f, 0.9f), 0.35f, Unit::Percent);
    F(P::dlyTone,   "Delay Tone", unit01, 0.6f, Unit::Percent);
    F(P::dlyMix,    "Delay Amount", unit01, 0.15f, Unit::Percent);

    B(P::revOn,     "Reverb On", true);
    F(P::revSize,   "Reverb Size", unit01, 0.6f, Unit::Percent);
    F(P::revDamp,   "Reverb Darkness", unit01, 0.5f, Unit::Percent);
    F(P::revPre,    "Reverb Pre-Delay", NR(0.0f, 120.0f), 15.0f, Unit::Ms);
    F(P::revMix,    "Reverb Amount", unit01, 0.18f, Unit::Percent);
    B(P::revGate,   "Reverb Gate", false);
    F(P::revGateTime,"Reverb Gate Time", skew(60.0f, 800.0f, 250.0f), 300.0f, Unit::Ms);

    // ---- Master -------------------------------------------------------------
    F(P::master,    "Master Volume", skew(-40.0f, 12.0f, -8.0f), -3.0f, Unit::DB);
    B(P::limOn,     "Limiter On", true);

    return layout;
}

} // namespace rl
