// ============================================================================
//  PresetManager.h  -  factory presets, user presets (XML files) and the
//                      "musical randomize" button.
// ============================================================================
#pragma once
#include <juce_audio_processors/juce_audio_processors.h>

namespace rl
{
class PresetManager
{
public:
    explicit PresetManager(juce::AudioProcessorValueTreeState& s) : apvts(s) {}

    // ---- factory presets ----
    static int getNumFactory();
    static juce::String getFactoryName(int index);
    void loadFactory(int index);

    // ---- user presets ----
    juce::File getUserFolder() const;
    juce::Array<juce::File> getUserPresetFiles() const;
    bool loadUserFile(const juce::File& file);
    bool saveUser(const juce::String& name);       // returns false if it could not write the file

    // ---- randomize (musical, never broken) ----
    void randomize();

    // ---- name shown at the top ----
    juce::String getCurrentName() const;
    void setCurrentName(const juce::String& name);

private:
    void resetToDefaults();
    void setReal(const juce::String& paramId, float realValue);

    juce::AudioProcessorValueTreeState& apvts;
};
} // namespace rl
