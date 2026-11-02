#include "PluginEditor.h"

using namespace rl;

//==============================================================================
//  KnobBox
//==============================================================================
KnobBox::KnobBox(juce::AudioProcessorValueTreeState& state, P::Id id, const juce::String& label,
                 const juce::String& tooltip, bool showValue_, bool showName_, float fontSize)
    : showValue(showValue_), showName(showName_)
{
    slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setRotaryParameters(juce::degreesToRadians(225.0f), juce::degreesToRadians(495.0f), true);
    slider.setScrollWheelEnabled(true);
    slider.setTooltip(tooltip);
    slider.setTextBoxStyle(showValue ? juce::Slider::TextBoxBelow : juce::Slider::NoTextBox, false, 64, 16);
    if (! showValue)
        slider.setPopupDisplayEnabled(true, true, nullptr);   // matrix knobs show their value while dragging
    attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(state, paramIds[id], slider);
    addAndMakeVisible(slider);

    if (showName)
    {
        nameLabel.setText(label, juce::dontSendNotification);
        nameLabel.setJustificationType(juce::Justification::centred);
        nameLabel.setFont(RageLookAndFeel::makeFont(fontSize, true));
        nameLabel.setColour(juce::Label::textColourId, fontSize > 12.0f ? rlcol::text : rlcol::textDim);
        nameLabel.setInterceptsMouseClicks(false, false);
        addAndMakeVisible(nameLabel);
    }
}

void KnobBox::resized()
{
    auto r = getLocalBounds();
    if (showName)
        nameLabel.setBounds(r.removeFromTop(14));
    if (showValue)
        slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, r.getWidth(), 16);
    slider.setBounds(r);
}

//==============================================================================
//  Content (the painted background)
//==============================================================================
void RageLeadEditor::Content::paint(juce::Graphics& g)
{
    g.fillAll(rlcol::background);

    // header strip
    g.setColour(juce::Colour(0xff101015));
    g.fillRect(0, 0, getWidth(), 60);
    g.setColour(rlcol::accent);
    g.fillRect(0, 60, getWidth(), 2);

    g.setFont(RageLookAndFeel::makeFont(32.0f, true));
    g.setColour(rlcol::accent);
    g.drawText("RAGE", 24, 6, 100, 40, juce::Justification::centredLeft, false);
    g.setColour(rlcol::text);
    g.drawText("LEAD", 118, 6, 100, 40, juce::Justification::centredLeft, false);
    g.setFont(RageLookAndFeel::makeFont(10.5f));
    g.setColour(rlcol::textDim);
    g.drawText("MELODIC RAGE SYNTH", 26, 40, 200, 14, juce::Justification::centredLeft, false);

    for (auto& p : panels)
    {
        g.setColour(rlcol::panel);
        g.fillRoundedRectangle(p.r.toFloat(), 10.0f);
        g.setColour(rlcol::panelEdge);
        g.drawRoundedRectangle(p.r.toFloat().reduced(0.5f), 10.0f, 1.0f);
        g.setColour(rlcol::accent);
        g.setFont(RageLookAndFeel::makeFont(11.5f, true));
        g.drawText(p.title, p.r.getX() + 16, p.r.getY() + 8, p.r.getWidth() - 32, 16, juce::Justification::centredLeft, false);
    }

    for (auto& c : captions)
    {
        g.setColour(rlcol::textDim);
        g.setFont(RageLookAndFeel::makeFont(c.size, true));
        g.drawText(c.text, c.r, c.just, false);
    }

    if (advanced)
    {
        g.setColour(rlcol::textDim);
        g.setFont(RageLookAndFeel::makeFont(12.5f));
        g.drawFittedText("Rows = where the movement comes from. Columns = what it changes.\n"
                         "Turn a knob right for positive, left for negative modulation.\n\n"
                         "Mod wheel: opens the filter and pushes the distortion.\n"
                         "MIDI note C0 (note 12): hold it for TAPE STOP.\n"
                         "Double-click any knob to reset it. Hover for help.",
                         juce::Rectangle<int>(780, 950, 540, 110), juce::Justification::topLeft, 6);
    }
}

//==============================================================================
//  Construction helpers
//==============================================================================
void RageLeadEditor::addKnob(P::Id id, const char* label, const char* tip, bool showValue, bool showName, float fontSize)
{
    auto k = std::make_unique<KnobBox>(proc.apvts, id, label, tip, showValue, showName, fontSize);
    content.addChildComponent(*k);
    knobs[(int) id] = std::move(k);
}

void RageLeadEditor::addCombo(P::Id id, const char* tip)
{
    auto item = std::make_unique<ComboItem>();
    if (auto* choice = dynamic_cast<juce::AudioParameterChoice*>(proc.apvts.getParameter(paramIds[id])))
        item->box.addItemList(choice->choices, 1);
    item->box.setTooltip(tip);
    item->att = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(proc.apvts, paramIds[id], item->box);
    content.addChildComponent(item->box);
    combos[(int) id] = std::move(item);
}

void RageLeadEditor::addToggle(P::Id id, const char* text, const char* tip)
{
    auto item = std::make_unique<ToggleItem>();
    item->btn.setButtonText(text);
    item->btn.setTooltip(tip);
    item->att = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(proc.apvts, paramIds[id], item->btn);
    content.addChildComponent(item->btn);
    toggles[(int) id] = std::move(item);
}

void RageLeadEditor::placeKnob(P::Id id, int x, int y, int d)
{
    auto& k = *knobs.at((int) id);
    const int h = k.neededHeight(d);
    const int w = (h == d) ? d : std::max(d + 8, 64);
    k.setBounds(x, y, w, h);
    k.setVisible(true);
}

void RageLeadEditor::placeCombo(P::Id id, int x, int y, int w, int h)
{
    auto& c = combos.at((int) id)->box;
    c.setBounds(x, y, w, h);
    c.setVisible(true);
}

void RageLeadEditor::placeToggle(P::Id id, int x, int y, int w, int h)
{
    auto& t = toggles.at((int) id)->btn;
    t.setBounds(x, y, w, h);
    t.setVisible(true);
}

void RageLeadEditor::caption(int x, int y, int w, const juce::String& text, juce::Justification j, float size)
{
    content.captions.push_back({ juce::Rectangle<int>(x, y, w, 14), text, j, size });
}

//==============================================================================
//  Constructor / destructor
//==============================================================================
RageLeadEditor::RageLeadEditor(RageLeadProcessor& p)
    : AudioProcessorEditor(&p), proc(p), scope(p)
{
    setLookAndFeel(&laf);
    advanced = (bool) proc.apvts.state.getProperty("advanced", false);
    uiScale = juce::jlimit(0.5f, 1.6f, (float) (double) proc.apvts.state.getProperty("uiScale", 0.8));

    addAndMakeVisible(content);

    //---- macros ---------------------------------------------------------------
    addKnob(P::macroRage,   "RAGE",   "RAGE macro: one knob that adds distortion, bit-crush and filter drive.", true, true, 15.0f);
    addKnob(P::macroWidth,  "WIDTH",  "WIDTH macro: adds unison voices, stereo spread and chorus.", true, true, 15.0f);
    addKnob(P::macroSpace,  "SPACE",  "SPACE macro: adds delay and reverb.", true, true, 15.0f);
    addKnob(P::macroBright, "BRIGHT", "BRIGHT macro: opens (right) or closes (left) the filter and the distortion tone. 50% = neutral.", true, true, 15.0f);

    //---- essentials -------------------------------------------------------------
    addKnob(P::cutoff,    "CUTOFF",    "Filter cutoff: turn right for a brighter sound, left for a darker one.");
    addKnob(P::res,       "RESONANCE", "Filter resonance: adds a whistling peak at the cutoff frequency.");
    addKnob(P::uniVoices, "UNISON",    "Unison: how many detuned copies of the sound are stacked (1 = off, 7 = huge).");
    addKnob(P::uniSpread, "SPREAD",    "Unison spread: how far apart the unison voices are detuned and panned.");
    addKnob(P::distDrive, "DRIVE",     "Distortion drive: how hard the sound is pushed into distortion.");
    addKnob(P::revMix,    "REVERB",    "Reverb amount.");
    addKnob(P::dlyMix,    "DELAY",     "Delay amount.");
    addKnob(P::master,    "MASTER",    "Master volume. The limiter keeps it from clipping.");

    //---- oscillators ---------------------------------------------------------------
    addCombo(P::o1wave, "Oscillator 1 waveform.");
    addCombo(P::o2wave, "Oscillator 2 waveform.");
    addCombo(P::voiceMode, "Poly = chords. Mono = one note at a time. Legato = mono that glides between notes without retriggering.");
    addKnob(P::o1level, "OSC1 LEVEL", "Volume of oscillator 1.");
    addKnob(P::o1pwm,   "PWM",        "Pulse width (Square / Chip Pulse waves). Changes how hollow or nasal the tone is.");
    addKnob(P::o2level, "OSC2 LEVEL", "Volume of oscillator 2.");
    addKnob(P::o2detune,"DETUNE",     "Fine-tune oscillator 2 against oscillator 1 (+/-100 cents) for a thick, wobbly sound.");
    addKnob(P::o2semi,  "SEMI",       "Transpose oscillator 2 in semitones (12 = one octave).");
    addKnob(P::subLevel,"SUB",        "Sub oscillator: a pure sine one octave down for weight.");
    addKnob(P::glide,   "GLIDE",      "Portamento: time to slide from one note to the next.");

    //---- filter ---------------------------------------------------------------------
    addCombo(P::fType, "Filter type: low-pass (darkens), high-pass (thins out), band-pass (nasal).");
    addKnob(P::fDrive,  "DRIVE",     "Pushes the signal into the filter for a gritty, saturated tone.");
    addKnob(P::keytrack,"KEYTRACK",  "How much the cutoff follows the note you play (100% = same brightness on every note).");
    addKnob(P::fEnvAmt, "ENV AMOUNT","How much the filter envelope opens (right) or closes (left) the filter on each note.");

    //---- envelopes ------------------------------------------------------------------
    const P::Id ampIds[4] = { P::aA, P::aD, P::aS, P::aR };
    const P::Id fltIds[4] = { P::fA, P::fD, P::fS, P::fR };
    const char* envNames[4] = { "ATTACK", "DECAY", "SUSTAIN", "RELEASE" };
    const char* ampTips[4] = { "Volume envelope attack: how fast the note fades in.", "Volume envelope decay: time to fall to the sustain level.",
                               "Volume envelope sustain: the level held while the key is down.", "Volume envelope release: fade-out time after you let go." };
    const char* fltTips[4] = { "Filter envelope attack.", "Filter envelope decay.", "Filter envelope sustain level.", "Filter envelope release." };
    for (int i = 0; i < 4; ++i)
    {
        addKnob(ampIds[i], envNames[i], ampTips[i]);
        addKnob(fltIds[i], envNames[i], fltTips[i]);
    }

    //---- LFOs -------------------------------------------------------------------------
    addCombo(P::l1shape, "LFO 1 shape."); addCombo(P::l1sync, "LFO 1 speed: free (Hz) or locked to the song tempo.");
    addCombo(P::l2shape, "LFO 2 shape."); addCombo(P::l2sync, "LFO 2 speed: free (Hz) or locked to the song tempo.");
    addKnob(P::l1rate, "RATE", "LFO 1 speed when set to Free.");
    addKnob(P::l2rate, "RATE", "LFO 2 speed when set to Free.");

    //---- FX ---------------------------------------------------------------------------
    addToggle(P::crushOn, "CRUSH", "Bit-crusher on/off.");
    addKnob(P::crushBits, "BITS", "Bit depth. Lower = grittier, more 8-bit.");
    addKnob(P::crushRate, "RATE", "Sample-rate reduction. Higher = more aliased and lo-fi.");
    addKnob(P::crushMix,  "AMOUNT", "How much crushed sound is mixed in.");

    addToggle(P::distOn, "DISTORTION", "Distortion on/off.");
    addCombo(P::distMode, "Distortion flavour: Hard Clip = brutal, Tube = warm and loud, Wavefold = metallic, Bit-Fold = glitchy.");
    addKnob(P::distTone, "TONE", "Tone of the distortion: left = dark, right = bright.");

    addToggle(P::stutOn, "STUTTER", "Stutter / glitch on/off.");
    addToggle(P::tapeStop, "TAPE STOP", "Hold to slow the sound down like a tape machine stopping. (MIDI note C0 does the same.)");
    addCombo(P::stutRate, "Slice length of the tempo-synced stutter.");
    addKnob(P::stutMix, "AMOUNT", "Stutter amount. 0 = off. Repeats slices of the sound in time with the song.");
    addKnob(P::tapeTime, "STOP TIME", "How long the tape stop takes to wind down.");

    addToggle(P::choOn, "CHORUS", "Chorus / ensemble on/off.");
    addKnob(P::choMix,  "AMOUNT", "Chorus amount: adds width.");
    addKnob(P::choRate, "RATE", "Chorus movement speed.");

    addToggle(P::dlyOn, "DELAY", "Ping-pong delay on/off.");
    addCombo(P::dlySync, "Delay time, locked to the song tempo.");
    addKnob(P::dlyFb,   "FEEDBACK", "How many echoes you hear.");
    addKnob(P::dlyTone, "TONE", "Brightness of the echoes.");

    addToggle(P::revOn, "REVERB", "Reverb on/off.");
    addToggle(P::revGate, "GATE", "Gated reverb: cuts the tail abruptly after the notes stop for a chopped rage tail.");
    addKnob(P::revSize, "SIZE", "Size of the room.");
    addKnob(P::revDamp, "DARK", "Makes the reverb darker.");
    addKnob(P::revPre,  "PRE-DELAY", "Gap before the reverb starts.");
    addKnob(P::revGateTime, "GATE TIME", "How long the reverb stays open when Gate is on.");

    addToggle(P::limOn, "LIMITER", "Output limiter on/off. A safety clipper always stays active so you never clip the master.");

    //---- mod matrix ---------------------------------------------------------------------
    const P::Id matrix[3][5] = {
        { P::l1Pitch, P::l1Cut, P::l1Pwm, P::l1Dist, P::l1Uni },
        { P::l2Pitch, P::l2Cut, P::l2Pwm, P::l2Dist, P::l2Uni },
        { P::ePitch,  P::eCut,  P::ePwm,  P::eDist,  P::eUni } };
    const char* srcNames[3] = { "LFO 1", "LFO 2", "Filter envelope" };
    const char* dstNames[5] = { "pitch", "filter cutoff", "pulse width", "distortion", "unison detune" };
    for (int s = 0; s < 3; ++s)
        for (int d = 0; d < 5; ++d)
        {
            const juce::String tip = juce::String(srcNames[s]) + " -> " + dstNames[d] + ". Turn right for positive, left for negative.";
            addKnob(matrix[s][d], "", tip.toRawUTF8(), false, false);
        }

    //---- header controls --------------------------------------------------------------
    content.addAndMakeVisible(presetBox);
    content.addAndMakeVisible(prevButton);
    content.addAndMakeVisible(nextButton);
    content.addAndMakeVisible(saveButton);
    content.addAndMakeVisible(randomButton);
    content.addAndMakeVisible(viewButton);
    content.addChildComponent(scope);

    presetBox.setTooltip("Choose a preset.");
    prevButton.setTooltip("Previous preset");
    nextButton.setTooltip("Next preset");
    saveButton.setTooltip("Save the current sound as your own preset.");
    randomButton.setTooltip("Make a new random (but musical) sound.");
    viewButton.setTooltip("Switch between BASIC (macros + essentials) and ADVANCED (every control).");

    presetBox.onChange = [this] { onPresetChosen(); };
    prevButton.onClick = [this] { stepPreset(-1); };
    nextButton.onClick = [this] { stepPreset(+1); };
    saveButton.onClick = [this] { askToSavePreset(); };
    randomButton.onClick = [this] { proc.presets.randomize(); };
    viewButton.setToggleState(advanced, juce::dontSendNotification);
    viewButton.onClick = [this]
    {
        advanced = viewButton.getToggleState();
        proc.apvts.state.setProperty("advanced", advanced, nullptr);
        applyMode();
    };

    refreshPresetList();
    proc.apvts.state.addListener(this);

    setResizable(true, true);
    layoutContent();
    setResizeLimits((int) (designW() * 0.5f), (int) (designH() * 0.5f), (int) (designW() * 1.6f), (int) (designH() * 1.6f));
    getConstrainer()->setFixedAspectRatio((double) designW() / (double) designH());
    setSize((int) std::round(designW() * uiScale), (int) std::round(designH() * uiScale));
}

RageLeadEditor::~RageLeadEditor()
{
    proc.apvts.state.removeListener(this);
    cancelPendingUpdate();
    setLookAndFeel(nullptr);
}

//==============================================================================
//  Layout
//==============================================================================
void RageLeadEditor::applyMode()
{
    layoutContent();
    setResizeLimits((int) (designW() * 0.5f), (int) (designH() * 0.5f), (int) (designW() * 1.6f), (int) (designH() * 1.6f));
    getConstrainer()->setFixedAspectRatio((double) designW() / (double) designH());
    setSize((int) std::round(designW() * uiScale), (int) std::round(designH() * uiScale));
}

void RageLeadEditor::resized()
{
    const float s = (float) getWidth() / (float) designW();
    content.setBounds(0, 0, designW(), designH());
    content.setTransform(juce::AffineTransform::scale(s));

    if (std::abs(s - uiScale) > 0.005f)
    {
        uiScale = s;
        proc.apvts.state.setProperty("uiScale", (double) s, nullptr);
    }
}

void RageLeadEditor::layoutContent()
{
    // hide everything, then show only what the current view needs
    for (auto& k : knobs)   k.second->setVisible(false);
    for (auto& c : combos)  c.second->box.setVisible(false);
    for (auto& t : toggles) t.second->btn.setVisible(false);
    scope.setVisible(false);
    content.panels.clear();
    content.captions.clear();
    content.advanced = advanced;
    content.designWidth = designW();
    content.setBounds(0, 0, designW(), designH());

    layoutHeader();
    if (advanced) layoutAdvanced(); else layoutBasic();
    content.repaint();
}

void RageLeadEditor::layoutHeader()
{
    const int W = designW();
    const int x0 = 270 + (W - 1000) / 2;
    prevButton.setBounds(x0, 12, 34, 34);
    presetBox.setBounds(x0 + 40, 12, 260, 34);
    nextButton.setBounds(x0 + 306, 12, 34, 34);
    saveButton.setBounds(x0 + 350, 12, 64, 34);
    randomButton.setBounds(x0 + 420, 12, 84, 34);
    viewButton.setBounds(W - 24 - 150, 12, 150, 34);
}

void RageLeadEditor::layoutBasic()
{
    content.panels.push_back({ { 20, 72, 620, 206 }, "MACROS  -  one knob, instant vibe" });
    content.panels.push_back({ { 652, 72, 328, 206 }, "SCOPE" });
    content.panels.push_back({ { 20, 290, 960, 250 }, "ESSENTIALS" });

    // macros
    const P::Id macros[4] = { P::macroRage, P::macroWidth, P::macroSpace, P::macroBright };
    for (int i = 0; i < 4; ++i)
        placeKnob(macros[i], 40 + i * 145 + 8, 72 + 36, 120);

    scope.setBounds(664, 104, 304, 162);
    scope.setVisible(true);

    // essentials: {id, diameter}
    const std::pair<P::Id, int> ess[8] = { { P::cutoff, 110 }, { P::res, 80 }, { P::uniVoices, 80 }, { P::uniSpread, 80 },
                                           { P::distDrive, 110 }, { P::revMix, 90 }, { P::dlyMix, 80 }, { P::master, 110 } };
    int total = 0;
    for (auto& e : ess) total += std::max(e.second + 8, 64);
    const int gap = (920 - total) / 7;
    int x = 40;
    for (auto& e : ess)
    {
        const int h = e.second + 32;
        placeKnob(e.first, x, 290 + 34 + (206 - h) / 2, e.second);
        x += std::max(e.second + 8, 64) + gap;
    }
}

void RageLeadEditor::layoutAdvanced()
{
    // ---------------- Row 1: macros / scope / master ----------------
    content.panels.push_back({ { 20, 72, 640, 192 }, "MACROS  -  one knob, instant vibe" });
    content.panels.push_back({ { 672, 72, 360, 192 }, "SCOPE" });
    content.panels.push_back({ { 1044, 72, 296, 192 }, "MASTER" });

    const P::Id macros[4] = { P::macroRage, P::macroWidth, P::macroSpace, P::macroBright };
    for (int i = 0; i < 4; ++i)
        placeKnob(macros[i], 40 + i * 152 + 20, 72 + 36, 104);

    scope.setBounds(684, 102, 336, 150);
    scope.setVisible(true);

    placeKnob(P::master, 1070, 72 + 36, 100);
    placeToggle(P::limOn, 1196, 124, 120, 34);
    caption(1190, 170, 140, "Safety clipper is always on.", juce::Justification::centredLeft, 10.0f);

    // ---------------- Row 2: oscillators / filter ----------------
    content.panels.push_back({ { 20, 274, 820, 200 }, "OSCILLATORS" });
    caption(36, 304, 140, "OSC 1 WAVE");
    caption(196, 304, 140, "OSC 2 WAVE");
    caption(356, 304, 120, "VOICE MODE");
    placeCombo(P::o1wave, 36, 320, 150);
    placeCombo(P::o2wave, 196, 320, 150);
    placeCombo(P::voiceMode, 356, 320, 120);

    const P::Id oscKnobs[9] = { P::o1level, P::o1pwm, P::o2level, P::o2detune, P::o2semi, P::subLevel,
                                P::uniVoices, P::uniSpread, P::glide };
    for (int i = 0; i < 9; ++i)
        placeKnob(oscKnobs[i], 36 + i * 84, 358, 64);

    content.panels.push_back({ { 852, 274, 488, 200 }, "FILTER" });
    caption(868, 304, 140, "TYPE");
    placeCombo(P::fType, 868, 320, 140, 26);
    placeKnob(P::cutoff, 868, 352, 88);
    placeKnob(P::res, 980, 366, 64);
    placeKnob(P::fDrive, 1064, 366, 64);
    placeKnob(P::keytrack, 1148, 366, 64);
    placeKnob(P::fEnvAmt, 1232, 366, 64);

    // ---------------- Row 3: envelopes / LFOs ----------------
    content.panels.push_back({ { 20, 484, 820, 170 }, "ENVELOPES" });
    caption(36, 516, 200, "AMP (VOLUME)");
    caption(430, 516, 200, "FILTER");
    const P::Id ampIds[4] = { P::aA, P::aD, P::aS, P::aR };
    const P::Id fltIds[4] = { P::fA, P::fD, P::fS, P::fR };
    for (int i = 0; i < 4; ++i)
    {
        placeKnob(ampIds[i], 36 + i * 90, 534, 60);
        placeKnob(fltIds[i], 430 + i * 90, 534, 60);
    }

    content.panels.push_back({ { 852, 484, 488, 170 }, "LFOs" });
    caption(868, 516, 100, "LFO 1");
    caption(1100, 516, 100, "LFO 2");
    caption(868, 534, 100, "SHAPE", juce::Justification::left, 9.5f);
    caption(868, 580, 100, "SPEED", juce::Justification::left, 9.5f);
    caption(1100, 534, 100, "SHAPE", juce::Justification::left, 9.5f);
    caption(1100, 580, 100, "SPEED", juce::Justification::left, 9.5f);
    placeCombo(P::l1shape, 868, 548, 120);
    placeCombo(P::l1sync, 868, 594, 120);
    placeKnob(P::l1rate, 1004, 534, 60);
    placeCombo(P::l2shape, 1100, 548, 120);
    placeCombo(P::l2sync, 1100, 594, 120);
    placeKnob(P::l2rate, 1236, 534, 60);

    // ---------------- Row 4: RAGE FX ----------------
    content.panels.push_back({ { 20, 664, 1320, 230 }, "RAGE FX" });

    // Crush
    placeToggle(P::crushOn, 36, 696, 80);
    placeKnob(P::crushBits, 36, 758, 56);
    placeKnob(P::crushRate, 104, 758, 56);
    placeKnob(P::crushMix, 172, 758, 56);

    // Distortion
    placeToggle(P::distOn, 256, 696, 110);
    placeCombo(P::distMode, 256, 726, 152);
    placeKnob(P::distDrive, 256, 754, 72);
    placeKnob(P::distTone, 344, 760, 56);

    // Stutter + tape stop
    placeToggle(P::stutOn, 424, 696, 70);
    placeToggle(P::tapeStop, 500, 696, 80);
    placeCombo(P::stutRate, 424, 726, 156);
    placeKnob(P::stutMix, 424, 758, 56);
    placeKnob(P::tapeTime, 492, 758, 56);

    // Chorus
    placeToggle(P::choOn, 596, 696, 80);
    placeKnob(P::choMix, 596, 758, 56);
    placeKnob(P::choRate, 664, 758, 56);

    // Delay
    placeToggle(P::dlyOn, 748, 696, 80);
    placeCombo(P::dlySync, 748, 726, 204);
    placeKnob(P::dlyFb, 748, 758, 56);
    placeKnob(P::dlyTone, 816, 758, 56);
    placeKnob(P::dlyMix, 884, 758, 56);

    // Reverb
    placeToggle(P::revOn, 968, 696, 80);
    placeToggle(P::revGate, 1054, 696, 60);
    placeKnob(P::revSize, 968, 758, 56);
    placeKnob(P::revDamp, 1036, 758, 56);
    placeKnob(P::revPre, 1104, 758, 56);
    placeKnob(P::revGateTime, 1172, 758, 56);
    placeKnob(P::revMix, 1240, 758, 56);

    // ---------------- Row 5: mod matrix ----------------
    content.panels.push_back({ { 20, 904, 1320, 180 }, "MOD MATRIX" });
    const char* cols[5] = { "PITCH", "CUTOFF", "PWM", "DISTORTION", "UNI DETUNE" };
    const char* rows[3] = { "LFO 1", "LFO 2", "FILTER ENV" };
    const int x0 = 170, step = 110;
    for (int c = 0; c < 5; ++c)
        caption(x0 + c * step, 936, step, cols[c], juce::Justification::centred);

    const P::Id matrix[3][5] = {
        { P::l1Pitch, P::l1Cut, P::l1Pwm, P::l1Dist, P::l1Uni },
        { P::l2Pitch, P::l2Cut, P::l2Pwm, P::l2Dist, P::l2Uni },
        { P::ePitch,  P::eCut,  P::ePwm,  P::eDist,  P::eUni } };
    for (int r = 0; r < 3; ++r)
    {
        caption(36, 962 + r * 40 + 11, 120, rows[r], juce::Justification::left, 11.0f);
        for (int c = 0; c < 5; ++c)
            placeKnob(matrix[r][c], x0 + c * step + (step - 36) / 2, 954 + r * 40, 36);
    }
}

//==============================================================================
//  Presets
//==============================================================================
void RageLeadEditor::refreshPresetList()
{
    presetBox.clear(juce::dontSendNotification);
    presetBox.addSectionHeading("Factory");
    for (int i = 0; i < PresetManager::getNumFactory(); ++i)
        presetBox.addItem(PresetManager::getFactoryName(i), 1 + i);

    userFiles = proc.presets.getUserPresetFiles();
    if (! userFiles.isEmpty())
    {
        presetBox.addSeparator();
        presetBox.addSectionHeading("User");
        for (int i = 0; i < userFiles.size(); ++i)
            presetBox.addItem(userFiles[i].getFileNameWithoutExtension(), 1000 + i);
    }
    presetBox.setText(proc.presets.getCurrentName(), juce::dontSendNotification);
}

void RageLeadEditor::onPresetChosen()
{
    const int id = presetBox.getSelectedId();
    if (id >= 1000)
    {
        if (id - 1000 < userFiles.size()) proc.presets.loadUserFile(userFiles[id - 1000]);
    }
    else if (id >= 1)
        proc.presets.loadFactory(id - 1);
}

void RageLeadEditor::stepPreset(int direction)
{
    std::vector<int> ids;
    for (int i = 0; i < presetBox.getNumItems(); ++i)
        if (presetBox.getItemId(i) > 0) ids.push_back(presetBox.getItemId(i));
    if (ids.empty()) return;

    int idx = -1;
    const int current = presetBox.getSelectedId();
    for (size_t i = 0; i < ids.size(); ++i)
        if (ids[i] == current) idx = (int) i;

    if (idx < 0) idx = direction > 0 ? -1 : 0;
    idx = (idx + direction + (int) ids.size()) % (int) ids.size();
    presetBox.setSelectedId(ids[(size_t) idx], juce::sendNotification);
}

void RageLeadEditor::askToSavePreset()
{
    auto* w = new juce::AlertWindow("Save preset", "Give your sound a name:", juce::MessageBoxIconType::NoIcon, this);
    w->addTextEditor("name", proc.presets.getCurrentName() == "Random" ? juce::String("My Preset") : proc.presets.getCurrentName());
    w->addButton("Save", 1, juce::KeyPress(juce::KeyPress::returnKey));
    w->addButton("Cancel", 0, juce::KeyPress(juce::KeyPress::escapeKey));

    juce::Component::SafePointer<RageLeadEditor> safe(this);
    w->enterModalState(true, juce::ModalCallbackFunction::create([safe, w](int result)
    {
        if (result == 1 && safe != nullptr)
        {
            safe->proc.presets.saveUser(w->getTextEditorContents("name"));
            safe->refreshPresetList();
        }
    }), true);
}

void RageLeadEditor::valueTreePropertyChanged(juce::ValueTree&, const juce::Identifier& property)
{
    if (property == juce::Identifier("presetName"))
        triggerAsyncUpdate();
}

void RageLeadEditor::handleAsyncUpdate()
{
    presetBox.setText(proc.presets.getCurrentName(), juce::dontSendNotification);
}
