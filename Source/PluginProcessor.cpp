#include "PluginProcessor.h"
#include "PluginEditor.h"

using namespace rl;

RageLeadProcessor::RageLeadProcessor()
    : AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "RAGELEAD", createLayout()),
      presets(apvts)
{
    for (int i = 0; i < P::Count; ++i)
        raw[(size_t) i] = apvts.getRawParameterValue(paramIds[i]);

    presets.loadFactory(0);   // the first preset sounds like a usable rage lead right away
}

bool RageLeadProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}

void RageLeadProcessor::prepareToPlay(double sampleRate, int)
{
    engine.prepare(sampleRate);
    crush.prepare(sampleRate);
    distortion.prepare(sampleRate, kMaxFxBlock);
    stutter.prepare(sampleRate);
    tapeStop.prepare(sampleRate);
    chorus.prepare(sampleRate);
    delay.prepare(sampleRate);
    reverb.prepare(sampleRate, kMaxFxBlock);
    output.prepare(sampleRate);
    scopeBuf.fill(0.0f);
}

//==============================================================================
// Collects all the knob values (plus the four MACRO knobs) into one struct.
void RageLeadProcessor::buildSynthParams(SynthParams& sp) const
{
    auto clamp01 = [](float x) { return std::min(std::max(x, 0.0f), 1.0f); };
    const float rage = get(P::macroRage), width = get(P::macroWidth), bright = get(P::macroBright);

    sp.o1wave = (int) get(P::o1wave);   sp.o2wave = (int) get(P::o2wave);
    sp.o1level = get(P::o1level);       sp.o1pwm = get(P::o1pwm);
    sp.o2level = get(P::o2level);       sp.o2detune = get(P::o2detune);
    sp.o2semi = get(P::o2semi);         sp.subLevel = get(P::subLevel);

    // WIDTH macro: more unison voices + wider spread
    sp.uniVoices = std::min(7, (int) std::lround(get(P::uniVoices) + width * 3.0f));
    sp.uniSpread = clamp01(get(P::uniSpread) + width * 0.45f);
    sp.voiceMode = (int) get(P::voiceMode);
    sp.glideMs = get(P::glide);

    // BRIGHT macro: moves the filter cutoff up (bright) or down (dark) by up to 2.5 octaves
    sp.fType = (int) get(P::fType);
    sp.cutoff = get(P::cutoff) * std::exp2((bright - 0.5f) * 5.0f);
    sp.res = get(P::res);
    sp.fDrive = clamp01(get(P::fDrive) + rage * 0.4f);   // RAGE macro also pushes the filter drive
    sp.keytrack = get(P::keytrack);
    sp.fEnvAmt = get(P::fEnvAmt);

    sp.aA = get(P::aA); sp.aD = get(P::aD); sp.aS = get(P::aS); sp.aR = get(P::aR);
    sp.fA = get(P::fA); sp.fD = get(P::fD); sp.fS = get(P::fS); sp.fR = get(P::fR);

    sp.lfoRate[0] = get(P::l1rate);  sp.lfoRate[1] = get(P::l2rate);
    sp.lfoSync[0] = (int) get(P::l1sync);  sp.lfoSync[1] = (int) get(P::l2sync);
    sp.lfoShape[0] = (int) get(P::l1shape); sp.lfoShape[1] = (int) get(P::l2shape);

    const P::Id matrix[3][5] = {
        { P::l1Pitch, P::l1Cut, P::l1Pwm, P::l1Dist, P::l1Uni },
        { P::l2Pitch, P::l2Cut, P::l2Pwm, P::l2Dist, P::l2Uni },
        { P::ePitch,  P::eCut,  P::ePwm,  P::eDist,  P::eUni } };
    for (int s = 0; s < 3; ++s)
        for (int d = 0; d < 5; ++d)
            sp.mod[s][d] = get(matrix[s][d]);

    sp.bpm = bpm;
    sp.ppq = ppq;
    sp.hostPlaying = hostPlaying && havePpq;
}

//==============================================================================
void RageLeadProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;
    const int numSamples = buffer.getNumSamples();
    buffer.clear();
    if (buffer.getNumChannels() < 2 || numSamples == 0)
        return;

    // ---- host tempo / position -------------------------------------------------
    bpm = 120.0; ppq = 0.0; hostPlaying = false; havePpq = false;
    if (auto* ph = getPlayHead())
        if (auto pos = ph->getPosition())
        {
            if (auto b = pos->getBpm(); b.hasValue() && *b > 20.0) bpm = *b;
            if (auto p = pos->getPpqPosition(); p.hasValue()) { ppq = *p; havePpq = true; }
            hostPlaying = pos->getIsPlaying();
        }

    SynthParams sp;
    buildSynthParams(sp);
    engine.setTarget(sp);

    float* L = buffer.getWritePointer(0);
    float* R = buffer.getWritePointer(1);

    // ---- render synth, splitting the block at every MIDI event ----------------
    int pos = 0;
    for (const auto metadata : midi)
    {
        const auto msg = metadata.getMessage();
        const int t = std::min(std::max(metadata.samplePosition, pos), numSamples);
        if (t > pos)
        {
            engine.render(L + pos, R + pos, t - pos);
            pos = t;
        }

        if (msg.isNoteOn())
        {
            if (msg.getNoteNumber() == kTapeStopNote) tapeNoteHeld = true;
            else engine.noteOn(msg.getNoteNumber(), msg.getFloatVelocity());
        }
        else if (msg.isNoteOff())
        {
            if (msg.getNoteNumber() == kTapeStopNote) tapeNoteHeld = false;
            else engine.noteOff(msg.getNoteNumber());
        }
        else if (msg.isController())
            engine.controller(msg.getControllerNumber(), msg.getControllerValue());
        else if (msg.isPitchWheel())
            engine.pitchWheel(msg.getPitchWheelValue());
        else if (msg.isAllNotesOff() || msg.isAllSoundOff())
            engine.allNotesOff();
    }
    if (pos < numSamples)
        engine.render(L + pos, R + pos, numSamples - pos);
    distModNow = engine.getDistMod();

    // ---- effects (in chunks, so we never exceed the pre-allocated buffers) ----
    for (int start = 0; start < numSamples; start += kMaxFxBlock)
        runEffects(L + start, R + start, std::min(kMaxFxBlock, numSamples - start));

    // ---- feed the display -------------------------------------------------------
    int w = scopeWrite.load(std::memory_order_relaxed);
    for (int i = 0; i < numSamples; ++i)
    {
        scopeBuf[(size_t) w] = 0.5f * (L[i] + R[i]);
        w = (w + 1) & (kScopeSize - 1);
    }
    scopeWrite.store(w, std::memory_order_release);

    // final safety: never send NaN/inf to the host
    for (int i = 0; i < numSamples; ++i)
    {
        if (! std::isfinite(L[i])) L[i] = 0.0f;
        if (! std::isfinite(R[i])) R[i] = 0.0f;
    }
}

// The RAGE chain:  Crush -> Distortion -> Stutter -> Tape stop -> Chorus -> Delay -> Reverb -> Limiter
void RageLeadProcessor::runEffects(float* L, float* R, int n)
{
    auto clamp01 = [](float x) { return std::min(std::max(x, 0.0f), 1.0f); };
    const float rage = get(P::macroRage), width = get(P::macroWidth);
    const float space = get(P::macroSpace), bright = get(P::macroBright);

    // RAGE: more crush + more distortion
    crush.process(L, R, n, std::max(3.0f, get(P::crushBits) - rage * 5.0f),
                  get(P::crushRate) + rage * 3.0f,
                  getBool(P::crushOn) ? clamp01(get(P::crushMix) + rage * 0.4f) : 0.0f);

    // BRIGHT also opens up the distortion tone; the LFO / envelope mod matrix adds to drive
    const float drive = clamp01(get(P::distDrive) + rage * 0.55f + distModNow * 0.5f);
    const float tone = clamp01(get(P::distTone) + (bright - 0.5f) * 0.8f);
    distortion.process(L, R, n, getBool(P::distOn), (int) get(P::distMode), drive, tone);

    const double stutBeats = stutRateBeats[std::min((int) get(P::stutRate), 6)];
    stutter.process(L, R, n, getBool(P::stutOn), get(P::stutMix), stutBeats, bpm,
                    havePpq ? ppq : -1.0, hostPlaying);
    tapeStop.process(L, R, n, getBool(P::tapeStop) || tapeNoteHeld, get(P::tapeTime));

    // WIDTH: more chorus
    chorus.process(L, R, n, getBool(P::choOn), clamp01(get(P::choMix) + width * 0.5f), get(P::choRate));

    // SPACE: more delay + more reverb
    const double delaySec = delaySyncBeats[std::min((int) get(P::dlySync), 7)] * 60.0 / bpm;
    delay.process(L, R, n, getBool(P::dlyOn), delaySec, get(P::dlyFb) + space * 0.15f,
                  get(P::dlyTone), clamp01(get(P::dlyMix) + space * 0.35f));
    reverb.process(L, R, n, getBool(P::revOn), clamp01(get(P::revSize) + space * 0.2f), get(P::revDamp),
                   get(P::revPre), clamp01(get(P::revMix) + space * 0.4f),
                   getBool(P::revGate), get(P::revGateTime));

    output.process(L, R, n, get(P::master), getBool(P::limOn));
}

//==============================================================================
juce::AudioProcessorEditor* RageLeadProcessor::createEditor() { return new RageLeadEditor(*this); }

void RageLeadProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    if (auto xml = apvts.copyState().createXml())
        copyXmlToBinary(*xml, destData);
}

void RageLeadProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary(data, sizeInBytes))
        if (xml->hasTagName(apvts.state.getType()))
            apvts.replaceState(juce::ValueTree::fromXml(*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new RageLeadProcessor(); }
