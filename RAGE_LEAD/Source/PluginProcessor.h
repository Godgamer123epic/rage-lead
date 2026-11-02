// ============================================================================
//  PluginProcessor.h  -  the "brain" of RAGE LEAD: receives MIDI, makes sound.
// ============================================================================
#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include <array>
#include <atomic>
#include "Parameters.h"
#include "PresetManager.h"
#include "Voice.h"
#include "Crush.h"
#include "Distortion.h"
#include "Stutter.h"
#include "Chorus.h"
#include "Delay.h"
#include "Reverb.h"
#include "Limiter.h"

class RageLeadProcessor : public juce::AudioProcessor
{
public:
    RageLeadProcessor();
    ~RageLeadProcessor() override = default;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "RAGE LEAD"; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 4.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState apvts;
    rl::PresetManager presets;

    // Audio shown on the little oscilloscope / spectrum in the editor
    static constexpr int kScopeSize = 2048;
    std::array<float, kScopeSize> scopeBuf {};
    std::atomic<int> scopeWrite { 0 };

private:
    float get(rl::P::Id id) const { return raw[(size_t) id]->load(); }
    bool  getBool(rl::P::Id id) const { return get(id) > 0.5f; }
    void  buildSynthParams(rl::SynthParams& sp) const;
    void  runEffects(float* L, float* R, int n);

    std::array<std::atomic<float>*, rl::P::Count> raw {};

    rl::SynthEngine engine;
    rl::Crush crush;
    rl::Distortion distortion;
    rl::Stutter stutter;
    rl::TapeStop tapeStop;
    rl::Chorus chorus;
    rl::PingPongDelay delay;
    rl::DarkReverb reverb;
    rl::OutputStage output;

    // per-block info from the host
    double bpm = 120.0, ppq = 0.0;
    bool hostPlaying = false, havePpq = false;
    bool tapeNoteHeld = false;
    float distModNow = 0.0f;

    static constexpr int kMaxFxBlock = 2048;
    static constexpr int kTapeStopNote = 12; // MIDI note C0 (= "C0" in FL Studio) triggers the tape stop

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RageLeadProcessor)
};
