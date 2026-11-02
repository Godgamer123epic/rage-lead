#include "PresetManager.h"
#include "Parameters.h"

namespace rl
{
namespace
{
struct FactoryPreset
{
    const char* name;
    std::vector<std::pair<const char*, float>> values; // parameter id -> real value
};

// Menu index cheat-sheet used below:
//  waves      : 0 Saw  1 Square  2 Triangle  3 Chip Pulse  4 Chip Saw  5 Bell
//  voiceMode  : 0 Poly  1 Mono  2 Legato
//  distMode   : 0 Hard Clip  1 Tube  2 Wavefold  3 Bit-Fold
//  lfo shape  : 0 Sine  1 Saw  2 Square  3 Random      lfo sync: 0 Free, 3 = 1/4, 4 = 1/8, 5 = 1/16
//  stutRate   : 3 = 1/16  4 = 1/16T          dlySync: 2 = 1/8, 3 = 1/8 dotted, 5 = 1/4
const std::vector<FactoryPreset>& factory()
{
    static const std::vector<FactoryPreset> presets = {

        { "Carti Alien Lead", {
            { "o1wave", 0 }, { "o1level", 0.8f }, { "o2wave", 1 }, { "o2level", 0.55f }, { "o2detune", 14 }, { "o2semi", 12 },
            { "uniVoices", 4 }, { "uniSpread", 0.5f }, { "cutoff", 7500 }, { "res", 0.32f }, { "fDrive", 0.3f },
            { "keytrack", 0.6f }, { "fEnvAmt", 0.3f }, { "aA", 1 }, { "aD", 400 }, { "aS", 0.75f }, { "aR", 220 },
            { "fD", 300 }, { "fS", 0.3f }, { "l1rate", 5.2f }, { "l1Pitch", 0.08f },
            { "crushMix", 0.1f }, { "crushBits", 10 }, { "distMode", 0 }, { "distDrive", 0.5f }, { "distTone", 0.62f },
            { "choMix", 0.35f }, { "dlySync", 3 }, { "dlyMix", 0.22f }, { "dlyFb", 0.4f }, { "dlyTone", 0.55f },
            { "revMix", 0.22f }, { "revSize", 0.65f }, { "revDamp", 0.55f },
            { "macroRage", 0.45f }, { "macroWidth", 0.4f }, { "macroSpace", 0.3f }, { "macroBright", 0.55f } } },

        { "Rage Bell", {
            { "o1wave", 5 }, { "o1level", 0.85f }, { "o2wave", 2 }, { "o2level", 0.35f }, { "o2detune", 7 }, { "o2semi", 12 },
            { "uniVoices", 2 }, { "uniSpread", 0.3f }, { "cutoff", 11000 }, { "res", 0.1f }, { "keytrack", 0.8f }, { "fEnvAmt", 0.1f },
            { "aA", 1 }, { "aD", 1400 }, { "aS", 0.0f }, { "aR", 1100 }, { "distMode", 1 }, { "distDrive", 0.25f },
            { "choMix", 0.3f }, { "dlySync", 5 }, { "dlyMix", 0.3f }, { "dlyFb", 0.45f },
            { "revMix", 0.35f }, { "revSize", 0.8f }, { "revDamp", 0.6f }, { "revPre", 25 },
            { "macroRage", 0.2f }, { "macroWidth", 0.3f }, { "macroSpace", 0.5f }, { "macroBright", 0.6f } } },

        { "Chiptune Scream", {
            { "voiceMode", 1 }, { "glide", 45 }, { "o1wave", 3 }, { "o1pwm", 0.5f }, { "o1level", 0.8f },
            { "o2wave", 4 }, { "o2level", 0.45f }, { "o2detune", 6 }, { "uniVoices", 1 }, { "uniSpread", 0 },
            { "cutoff", 12000 }, { "res", 0.15f }, { "keytrack", 1.0f }, { "fEnvAmt", 0.1f },
            { "aA", 1 }, { "aD", 200 }, { "aS", 0.85f }, { "aR", 80 }, { "l1rate", 6 }, { "l1Pitch", 0.1f },
            { "crushBits", 8 }, { "crushRate", 2.5f }, { "crushMix", 0.55f },
            { "distMode", 3 }, { "distDrive", 0.45f }, { "distTone", 0.8f }, { "choMix", 0.15f },
            { "dlySync", 0 }, { "dlyMix", 0.2f }, { "dlyFb", 0.3f }, { "revMix", 0.1f },
            { "macroRage", 0.5f }, { "macroWidth", 0 }, { "macroSpace", 0.2f }, { "macroBright", 0.6f } } },

        { "Dark Pluck", {
            { "o1wave", 0 }, { "o2wave", 1 }, { "o1pwm", 0.35f }, { "o2level", 0.5f }, { "o2detune", 10 }, { "subLevel", 0.3f },
            { "uniVoices", 2 }, { "uniSpread", 0.25f }, { "cutoff", 1000 }, { "res", 0.35f }, { "fEnvAmt", 0.75f }, { "keytrack", 0.5f },
            { "fD", 180 }, { "fS", 0.0f }, { "aA", 1 }, { "aD", 260 }, { "aS", 0.0f }, { "aR", 160 },
            { "distMode", 1 }, { "distDrive", 0.3f }, { "dlySync", 2 }, { "dlyMix", 0.12f },
            { "revGate", 1 }, { "revGateTime", 350 }, { "revMix", 0.3f }, { "revSize", 0.7f }, { "revDamp", 0.7f },
            { "macroRage", 0.15f }, { "macroWidth", 0.1f }, { "macroSpace", 0.2f }, { "macroBright", 0.4f } } },

        { "Pierce the Veil Lead", {
            { "voiceMode", 2 }, { "glide", 60 }, { "o1wave", 0 }, { "o2wave", 0 }, { "o2detune", 18 }, { "o2level", 0.7f },
            { "uniVoices", 7 }, { "uniSpread", 0.65f }, { "cutoff", 10500 }, { "res", 0.18f }, { "keytrack", 0.5f }, { "fEnvAmt", 0.2f },
            { "aA", 3 }, { "aD", 500 }, { "aS", 0.8f }, { "aR", 260 }, { "l1rate", 5.5f }, { "l1Pitch", 0.12f },
            { "distMode", 0 }, { "distDrive", 0.45f }, { "choMix", 0.55f }, { "dlySync", 5 }, { "dlyMix", 0.28f }, { "dlyFb", 0.42f },
            { "revMix", 0.3f }, { "revSize", 0.85f },
            { "macroRage", 0.4f }, { "macroWidth", 0.7f }, { "macroSpace", 0.5f }, { "macroBright", 0.65f } } },

        { "Distorted 808 Stab", {
            { "voiceMode", 1 }, { "glide", 70 }, { "o1wave", 1 }, { "o1pwm", 0.35f }, { "o1level", 0.8f },
            { "o2wave", 0 }, { "o2semi", -12 }, { "o2level", 0.7f }, { "o2detune", 0 }, { "subLevel", 0.8f },
            { "uniVoices", 1 }, { "uniSpread", 0 }, { "cutoff", 2600 }, { "res", 0.25f }, { "fEnvAmt", 0.55f },
            { "fD", 220 }, { "fS", 0.1f }, { "aA", 1 }, { "aD", 240 }, { "aS", 0.35f }, { "aR", 110 },
            { "distMode", 0 }, { "distDrive", 0.8f }, { "distTone", 0.5f }, { "choMix", 0.1f },
            { "revGate", 1 }, { "revGateTime", 220 }, { "revMix", 0.22f }, { "dlyMix", 0.08f },
            { "macroRage", 0.65f }, { "macroWidth", 0 }, { "macroSpace", 0.15f }, { "macroBright", 0.4f } } },

        { "Red Siren", {
            { "voiceMode", 2 }, { "glide", 140 }, { "o1wave", 0 }, { "o2wave", 0 }, { "o2detune", 25 }, { "o2level", 0.75f },
            { "uniVoices", 3 }, { "uniSpread", 0.45f }, { "cutoff", 5200 }, { "res", 0.45f }, { "fDrive", 0.35f },
            { "fEnvAmt", 0.1f }, { "keytrack", 0.6f }, { "aA", 8 }, { "aD", 800 }, { "aS", 0.9f }, { "aR", 400 },
            { "l1rate", 0.9f }, { "l1Pitch", 0.25f }, { "l2shape", 1 }, { "l2sync", 2 }, { "l2Cut", 0.3f },
            { "distMode", 1 }, { "distDrive", 0.5f }, { "choMix", 0.4f }, { "dlySync", 2 }, { "dlyMix", 0.3f },
            { "revMix", 0.3f }, { "macroRage", 0.35f }, { "macroWidth", 0.5f }, { "macroSpace", 0.45f }, { "macroBright", 0.5f } } },

        { "Glitch Choir", {
            { "o1wave", 5 }, { "o2wave", 2 }, { "o2semi", 12 }, { "o2detune", 12 }, { "o2level", 0.5f },
            { "uniVoices", 5 }, { "uniSpread", 0.6f }, { "cutoff", 6000 }, { "res", 0.2f },
            { "aA", 40 }, { "aD", 800 }, { "aS", 0.6f }, { "aR", 600 },
            { "stutRate", 4 }, { "stutMix", 0.65f }, { "distMode", 2 }, { "distDrive", 0.25f }, { "distTone", 0.6f },
            { "choMix", 0.5f }, { "dlyMix", 0.3f }, { "dlyFb", 0.5f }, { "revMix", 0.4f }, { "revSize", 0.85f },
            { "macroRage", 0.3f }, { "macroWidth", 0.6f }, { "macroSpace", 0.6f }, { "macroBright", 0.55f } } },

        { "Melodic Haze", {
            { "o1wave", 2 }, { "o2wave", 0 }, { "o2semi", 12 }, { "o2level", 0.3f }, { "o2detune", 9 },
            { "uniVoices", 3 }, { "uniSpread", 0.4f }, { "cutoff", 2400 }, { "res", 0.15f }, { "keytrack", 0.6f }, { "fEnvAmt", 0.2f },
            { "aA", 15 }, { "aD", 600 }, { "aS", 0.6f }, { "aR", 500 }, { "l1rate", 4 }, { "l1Cut", 0.15f },
            { "distMode", 1 }, { "distDrive", 0.1f }, { "choMix", 0.75f }, { "choRate", 0.4f },
            { "dlySync", 3 }, { "dlyMix", 0.35f }, { "dlyFb", 0.5f }, { "dlyTone", 0.4f },
            { "revMix", 0.45f }, { "revSize", 0.9f }, { "revDamp", 0.6f },
            { "macroRage", 0 }, { "macroWidth", 0.6f }, { "macroSpace", 0.7f }, { "macroBright", 0.45f } } },

        { "Hyperpop Pluck", {
            { "o1wave", 4 }, { "o2wave", 1 }, { "o2semi", 12 }, { "o2level", 0.5f },
            { "uniVoices", 3 }, { "uniSpread", 0.3f }, { "cutoff", 8000 }, { "res", 0.3f }, { "fEnvAmt", 0.5f }, { "fD", 140 }, { "fS", 0.1f },
            { "aA", 1 }, { "aD", 220 }, { "aS", 0.0f }, { "aR", 120 }, { "crushMix", 0.25f }, { "crushBits", 10 },
            { "distMode", 1 }, { "distDrive", 0.4f }, { "dlySync", 0 }, { "dlyMix", 0.25f }, { "revMix", 0.15f },
            { "macroRage", 0.35f }, { "macroWidth", 0.3f }, { "macroSpace", 0.3f }, { "macroBright", 0.7f } } },

        { "Mosh Pit Saw", {
            { "o1wave", 0 }, { "o2wave", 0 }, { "o2detune", 30 }, { "o2level", 0.8f },
            { "uniVoices", 7 }, { "uniSpread", 0.8f }, { "cutoff", 4800 }, { "res", 0.4f }, { "fDrive", 0.5f },
            { "aA", 1 }, { "aD", 300 }, { "aS", 0.8f }, { "aR", 100 }, { "crushMix", 0.2f }, { "crushBits", 9 },
            { "distMode", 0 }, { "distDrive", 0.85f }, { "distTone", 0.5f }, { "choMix", 0.2f }, { "dlyMix", 0.1f }, { "revMix", 0.12f },
            { "macroRage", 0.8f }, { "macroWidth", 0.5f }, { "macroSpace", 0.1f }, { "macroBright", 0.5f } } },

        { "Tape Stop Bells", {
            { "o1wave", 5 }, { "o2wave", 2 }, { "o2semi", 7 }, { "o2level", 0.3f }, { "uniVoices", 2 }, { "uniSpread", 0.2f },
            { "cutoff", 9000 }, { "aA", 1 }, { "aD", 1000 }, { "aS", 0.15f }, { "aR", 700 }, { "tapeTime", 900 },
            { "stutMix", 0.2f }, { "distDrive", 0.2f }, { "dlySync", 2 }, { "dlyMix", 0.3f }, { "revMix", 0.35f }, { "revSize", 0.75f },
            { "macroRage", 0.15f }, { "macroWidth", 0.3f }, { "macroSpace", 0.4f }, { "macroBright", 0.55f } } },

        { "Acid Wobble Lead", {
            { "voiceMode", 1 }, { "glide", 30 }, { "o1wave", 0 }, { "o2wave", 1 }, { "o1pwm", 0.4f }, { "o2detune", 7 },
            { "uniVoices", 2 }, { "uniSpread", 0.2f }, { "cutoff", 1800 }, { "res", 0.6f }, { "fDrive", 0.4f }, { "fEnvAmt", 0.2f },
            { "l2shape", 1 }, { "l2sync", 4 }, { "l2Cut", 0.55f }, { "distMode", 1 }, { "distDrive", 0.55f },
            { "choMix", 0.2f }, { "dlySync", 3 }, { "dlyMix", 0.2f }, { "revMix", 0.15f },
            { "macroRage", 0.5f }, { "macroWidth", 0.2f }, { "macroSpace", 0.25f }, { "macroBright", 0.5f } } },

        { "Plain Init Saw", {
            { "distDrive", 0.1f }, { "choMix", 0.1f }, { "dlyMix", 0.0f }, { "revMix", 0.1f },
            { "macroBright", 0.5f } } },
    };
    return presets;
}
} // namespace

//==============================================================================
int PresetManager::getNumFactory() { return (int) factory().size(); }

juce::String PresetManager::getFactoryName(int index)
{
    if (index < 0 || index >= getNumFactory()) return {};
    return factory()[(size_t) index].name;
}

void PresetManager::setReal(const juce::String& paramId, float realValue)
{
    if (auto* p = apvts.getParameter(paramId))
        p->setValueNotifyingHost(p->convertTo0to1(realValue));
}

// Put every parameter (except master volume) back to its default value.
void PresetManager::resetToDefaults()
{
    for (int i = 0; i < P::Count; ++i)
    {
        if (i == P::master) continue;
        if (auto* p = apvts.getParameter(paramIds[i]))
            p->setValueNotifyingHost(p->getDefaultValue());
    }
}

void PresetManager::loadFactory(int index)
{
    if (index < 0 || index >= getNumFactory()) return;
    resetToDefaults();
    for (auto& kv : factory()[(size_t) index].values)
        setReal(kv.first, kv.second);
    setCurrentName(factory()[(size_t) index].name);
}

//==============================================================================
juce::File PresetManager::getUserFolder() const
{
    auto dir = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
                   .getChildFile("RAGE LEAD").getChildFile("Presets");
    dir.createDirectory();
    return dir;
}

juce::Array<juce::File> PresetManager::getUserPresetFiles() const
{
    auto files = getUserFolder().findChildFiles(juce::File::findFiles, false, "*.xml");
    files.sort();
    return files;
}

bool PresetManager::saveUser(const juce::String& rawName)
{
    auto name = rawName.trim();
    if (name.isEmpty()) name = "User Preset";
    auto safe = juce::File::createLegalFileName(name);

    juce::XmlElement root("RAGELEADPRESET");
    root.setAttribute("name", name);
    for (int i = 0; i < P::Count; ++i)
    {
        auto* e = root.createNewChildElement("P");
        e->setAttribute("id", paramIds[i]);
        e->setAttribute("v", (double) apvts.getRawParameterValue(paramIds[i])->load());
    }

    const bool ok = root.writeTo(getUserFolder().getChildFile(safe + ".xml"));
    if (ok) setCurrentName(name);
    return ok;
}

bool PresetManager::loadUserFile(const juce::File& file)
{
    auto xml = juce::XmlDocument::parse(file);
    if (xml == nullptr || ! xml->hasTagName("RAGELEADPRESET"))
        return false;

    resetToDefaults();
    for (auto* e : xml->getChildWithTagNameIterator("P"))
    {
        const auto id = e->getStringAttribute("id");
        
        setReal(id, (float) e->getDoubleAttribute("v"));
    }
    setCurrentName(xml->getStringAttribute("name", file.getFileNameWithoutExtension()));
    return true;
}

//==============================================================================
juce::String PresetManager::getCurrentName() const
{
    return apvts.state.getProperty("presetName", "Init").toString();
}

void PresetManager::setCurrentName(const juce::String& name)
{
    apvts.state.setProperty("presetName", name, nullptr);
}

//==============================================================================
// Randomize: every value is picked from a hand-tuned "musical" range, so the
// result is always playable (never silent, never painfully loud, never broken).
void PresetManager::randomize()
{
    juce::Random r;
    auto rf = [&](float lo, float hi) { return lo + r.nextFloat() * (hi - lo); };
    auto logf = [&](float lo, float hi) { return std::exp2(rf(std::log2(lo), std::log2(hi))); };
    auto pick = [&](std::initializer_list<int> items) { return *(items.begin() + r.nextInt((int) items.size())); };
    auto chance = [&](float p) { return r.nextFloat() < p; };

    resetToDefaults();

    setReal("o1wave", (float) pick({ 0, 0, 0, 1, 3, 4, 5 }));
    setReal("o2wave", (float) pick({ 0, 1, 2, 4 }));
    setReal("o1level", rf(0.7f, 0.9f));
    setReal("o2level", rf(0.3f, 0.7f));
    setReal("o1pwm", rf(0.3f, 0.7f));
    setReal("o2detune", rf(-22.0f, 22.0f));
    setReal("o2semi", (float) pick({ 0, 0, 0, 12, -12, 7 }));
    setReal("subLevel", chance(0.3f) ? rf(0.2f, 0.6f) : 0.0f);
    setReal("uniVoices", (float) pick({ 1, 2, 3, 3, 4, 5, 5, 7 }));
    setReal("uniSpread", rf(0.15f, 0.7f));

    const bool mono = chance(0.3f);
    setReal("voiceMode", mono ? (float) pick({ 1, 2 }) : 0.0f);
    setReal("glide", mono ? rf(20.0f, 150.0f) : 0.0f);

    setReal("fType", 0.0f);
    setReal("cutoff", logf(1500.0f, 14000.0f));
    setReal("res", rf(0.08f, 0.45f));
    setReal("fDrive", rf(0.0f, 0.5f));
    setReal("fEnvAmt", rf(-0.1f, 0.6f));
    setReal("keytrack", rf(0.3f, 0.8f));

    const float sustain = chance(0.35f) ? rf(0.0f, 0.15f) : rf(0.5f, 0.95f);
    setReal("aA", logf(0.8f, 12.0f));
    setReal("aD", sustain < 0.2f ? rf(250.0f, 1200.0f) : rf(150.0f, 900.0f));
    setReal("aS", sustain);
    setReal("aR", rf(80.0f, 500.0f));
    setReal("fA", logf(0.8f, 6.0f));
    setReal("fD", rf(100.0f, 600.0f));
    setReal("fS", rf(0.0f, 0.5f));
    setReal("fR", rf(80.0f, 400.0f));

    setReal("l1rate", rf(3.5f, 7.0f));
    if (chance(0.5f)) setReal("l1Pitch", rf(0.04f, 0.14f));
    if (chance(0.4f))
    {
        setReal("l2shape", (float) pick({ 0, 1, 3 }));
        setReal("l2sync", (float) pick({ 3, 4, 5 }));
        setReal("l2Cut", rf(-0.35f, 0.35f));
    }

    setReal("crushMix", chance(0.4f) ? rf(0.1f, 0.4f) : 0.0f);
    setReal("crushBits", rf(7.0f, 14.0f));
    setReal("crushRate", rf(1.0f, 4.0f));
    setReal("distMode", (float) pick({ 0, 1, 1, 2, 3 }));
    setReal("distDrive", rf(0.25f, 0.8f));
    setReal("distTone", rf(0.45f, 0.85f));
    setReal("stutMix", chance(0.2f) ? rf(0.3f, 0.7f) : 0.0f);
    setReal("stutRate", (float) pick({ 3, 4, 5 }));
    setReal("choMix", rf(0.1f, 0.6f));
    setReal("dlySync", (float) pick({ 2, 3, 5 }));
    setReal("dlyMix", rf(0.05f, 0.35f));
    setReal("dlyFb", rf(0.25f, 0.55f));
    setReal("revMix", rf(0.1f, 0.4f));
    setReal("revSize", rf(0.4f, 0.9f));
    setReal("revDamp", rf(0.3f, 0.8f));
    setReal("revGate", chance(0.15f) ? 1.0f : 0.0f);

    setReal("macroRage", rf(0.0f, 0.6f));
    setReal("macroWidth", rf(0.0f, 0.6f));
    setReal("macroSpace", rf(0.0f, 0.5f));
    setReal("macroBright", rf(0.35f, 0.7f));

    setCurrentName("Random");
}
} // namespace rl
