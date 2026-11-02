// ============================================================================
//  PluginEditor.h  -  the user interface of RAGE LEAD
//
//  The whole interface is drawn on a fixed-size "design canvas" (Content) that
//  is scaled up/down to fit the window, so it never breaks when you resize.
//  BASIC view shows macros + the essential knobs, ADVANCED shows everything.
// ============================================================================
#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <map>
#include "PluginProcessor.h"
#include "LookAndFeel.h"
#include "ScopeComponent.h"

//------------------------------------------------------------------------------
// A rotary knob with a name above and a value readout below
//------------------------------------------------------------------------------
class KnobBox : public juce::Component
{
public:
    KnobBox(juce::AudioProcessorValueTreeState& state, rl::P::Id id, const juce::String& label,
            const juce::String& tooltip, bool showValue, bool showName, float fontSize);

    void resized() override;
    int  neededHeight(int diameter) const { return diameter + (showName ? 14 : 0) + (showValue ? 18 : 0); }

private:
    bool showValue, showName;
    juce::Label nameLabel;
    juce::Slider slider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
};

//------------------------------------------------------------------------------
class RageLeadEditor : public juce::AudioProcessorEditor,
                       private juce::ValueTree::Listener,
                       private juce::AsyncUpdater
{
public:
    explicit RageLeadEditor(RageLeadProcessor&);
    ~RageLeadEditor() override;

    void resized() override;

private:
    // the scaled drawing canvas
    struct Panel   { juce::Rectangle<int> r; juce::String title; };
    struct Caption { juce::Rectangle<int> r; juce::String text; juce::Justification just; float size; };
    class Content : public juce::Component
    {
    public:
        void paint(juce::Graphics&) override;
        std::vector<Panel> panels;
        std::vector<Caption> captions;
        bool advanced = false;
        int designWidth = 1000;
    };

    struct ComboItem  { juce::ComboBox box; std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> att; };
    struct ToggleItem { juce::ToggleButton btn; std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> att; };

    // building helpers
    void addKnob(rl::P::Id id, const char* label, const char* tip, bool showValue = true, bool showName = true, float fontSize = 11.0f);
    void addCombo(rl::P::Id id, const char* tip);
    void addToggle(rl::P::Id id, const char* text, const char* tip);
    void placeKnob(rl::P::Id id, int x, int y, int diameter);
    void placeCombo(rl::P::Id id, int x, int y, int w, int h = 26);
    void placeToggle(rl::P::Id id, int x, int y, int w, int h = 24);
    void caption(int x, int y, int w, const juce::String& text, juce::Justification j = juce::Justification::left, float size = 10.5f);

    // layout
    int  designW() const { return advanced ? 1360 : 1000; }
    int  designH() const { return advanced ? 1100 : 560; }
    void layoutContent();
    void layoutBasic();
    void layoutAdvanced();
    void layoutHeader();
    void applyMode();

    // presets
    void refreshPresetList();
    void stepPreset(int direction);
    void onPresetChosen();
    void askToSavePreset();

    // listeners
    void valueTreePropertyChanged(juce::ValueTree&, const juce::Identifier&) override;
    void handleAsyncUpdate() override;

    RageLeadProcessor& proc;
    RageLookAndFeel laf;
    juce::TooltipWindow tooltipWindow { this, 600 };

    Content content;
    std::map<int, std::unique_ptr<KnobBox>> knobs;
    std::map<int, std::unique_ptr<ComboItem>> combos;
    std::map<int, std::unique_ptr<ToggleItem>> toggles;
    ScopeComponent scope;

    juce::TextButton prevButton { "<" }, nextButton { ">" }, saveButton { "SAVE" }, randomButton { "RANDOM" };
    juce::ComboBox presetBox;
    juce::ToggleButton viewButton { "ADVANCED" };

    bool advanced = false;
    float uiScale = 0.8f;
    juce::Array<juce::File> userFiles;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RageLeadEditor)
};
