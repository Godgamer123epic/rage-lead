// ============================================================================
//  LookAndFeel.h  -  the red / black "rage" look: knobs, buttons, menus.
// ============================================================================
#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

namespace rlcol
{
const juce::Colour background { 0xff0a0a0d };
const juce::Colour panel      { 0xff141419 };
const juce::Colour panelEdge  { 0xff2a2a33 };
const juce::Colour knobBody   { 0xff1d1d24 };
const juce::Colour track      { 0xff2c2c36 };
const juce::Colour accent     { 0xffe5192f };
const juce::Colour accentHot  { 0xffff4a5c };
const juce::Colour text       { 0xffe2e2e8 };
const juce::Colour textDim    { 0xff8d8d99 };
}

class RageLookAndFeel : public juce::LookAndFeel_V4
{
public:
    RageLookAndFeel()
    {
        setColour(juce::Slider::textBoxTextColourId, rlcol::text);
        setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
        setColour(juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
        setColour(juce::Slider::textBoxHighlightColourId, rlcol::accent);
        setColour(juce::Label::textColourId, rlcol::text);
        setColour(juce::ComboBox::backgroundColourId, rlcol::knobBody);
        setColour(juce::ComboBox::textColourId, rlcol::text);
        setColour(juce::ComboBox::outlineColourId, rlcol::panelEdge);
        setColour(juce::ComboBox::arrowColourId, rlcol::accent);
        setColour(juce::PopupMenu::backgroundColourId, juce::Colour(0xff111116));
        setColour(juce::PopupMenu::textColourId, rlcol::text);
        setColour(juce::PopupMenu::highlightedBackgroundColourId, rlcol::accent);
        setColour(juce::PopupMenu::highlightedTextColourId, juce::Colours::white);
        setColour(juce::TextButton::buttonColourId, rlcol::knobBody);
        setColour(juce::TextButton::textColourOffId, rlcol::text);
        setColour(juce::TooltipWindow::backgroundColourId, juce::Colour(0xff1b1b22));
        setColour(juce::TooltipWindow::textColourId, rlcol::text);
        setColour(juce::TooltipWindow::outlineColourId, rlcol::accent);
        setColour(juce::TextEditor::backgroundColourId, rlcol::knobBody);
        setColour(juce::TextEditor::textColourId, rlcol::text);
    }

    static juce::Font makeFont(float size, bool bold = false)
    {
        return juce::Font(juce::FontOptions(size, bold ? juce::Font::bold : juce::Font::plain));
    }

    juce::Font getLabelFont(juce::Label&) override        { return makeFont(12.0f); }
    juce::Font getComboBoxFont(juce::ComboBox&) override  { return makeFont(13.0f); }
    juce::Font getPopupMenuFont() override                { return makeFont(14.0f); }
    juce::Font getTextButtonFont(juce::TextButton&, int) override { return makeFont(13.0f, true); }

    //--------------------------------------------------------------------------
    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
                          float startAngle, float endAngle, juce::Slider& slider) override
    {
        auto bounds = juce::Rectangle<float>((float) x, (float) y, (float) width, (float) height).reduced(2.0f);
        const float diameter = std::min(bounds.getWidth(), bounds.getHeight());
        const auto centre = bounds.getCentre();
        const float radius = diameter * 0.5f;
        const float thick = std::max(2.5f, radius * 0.13f);
        const float arcR = radius - thick * 0.5f - 1.0f;
        const float angle = startAngle + sliderPos * (endAngle - startAngle);
        const bool enabled = slider.isEnabled();

        // dark groove
        juce::Path groove;
        groove.addCentredArc(centre.x, centre.y, arcR, arcR, 0.0f, startAngle, endAngle, true);
        g.setColour(rlcol::track);
        g.strokePath(groove, juce::PathStrokeType(thick, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        // lit value arc (from the middle for +/- knobs)
        float fromAngle = startAngle;
        if (slider.getMinimum() < 0.0 && slider.getMaximum() > 0.0)
            fromAngle = startAngle + (float) slider.valueToProportionOfLength(0.0) * (endAngle - startAngle);
        if (std::abs(angle - fromAngle) > 0.01f)
        {
            juce::Path val;
            val.addCentredArc(centre.x, centre.y, arcR, arcR, 0.0f, std::min(fromAngle, angle), std::max(fromAngle, angle), true);
            g.setColour(rlcol::accent.withAlpha(0.25f));
            g.strokePath(val, juce::PathStrokeType(thick + 4.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
            g.setColour(enabled ? rlcol::accentHot : rlcol::textDim);
            g.strokePath(val, juce::PathStrokeType(thick, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        }

        // knob body
        const float bodyR = radius - thick - 4.0f;
        juce::ColourGradient grad(juce::Colour(0xff32323d), centre.x, centre.y - bodyR,
                                  juce::Colour(0xff121217), centre.x, centre.y + bodyR, false);
        g.setGradientFill(grad);
        g.fillEllipse(centre.x - bodyR, centre.y - bodyR, bodyR * 2.0f, bodyR * 2.0f);
        g.setColour(juce::Colour(0xff050507));
        g.drawEllipse(centre.x - bodyR, centre.y - bodyR, bodyR * 2.0f, bodyR * 2.0f, 1.2f);

        // pointer
        juce::Path p;
        const float pLen = bodyR * 0.72f, pW = std::max(2.0f, bodyR * 0.11f);
        p.addRoundedRectangle(-pW * 0.5f, -bodyR + 3.0f, pW, pLen * 0.55f, pW * 0.5f);
        g.setColour(rlcol::text);
        g.fillPath(p, juce::AffineTransform::rotation(angle).translated(centre.x, centre.y));
    }

    //--------------------------------------------------------------------------
    void drawToggleButton(juce::Graphics& g, juce::ToggleButton& b, bool highlighted, bool) override
    {
        auto r = b.getLocalBounds().toFloat().reduced(1.5f);
        const bool on = b.getToggleState();
        if (on)
        {
            g.setColour(rlcol::accent.withAlpha(0.28f));
            g.fillRoundedRectangle(r.expanded(1.5f), 6.0f);
        }
        g.setColour(on ? rlcol::accent : (highlighted ? juce::Colour(0xff2a2a33) : rlcol::knobBody));
        g.fillRoundedRectangle(r, 5.0f);
        g.setColour(on ? rlcol::accentHot : rlcol::panelEdge);
        g.drawRoundedRectangle(r, 5.0f, 1.2f);

        // little LED dot
        const float d = 7.0f;
        g.setColour(on ? juce::Colours::white : juce::Colour(0xff4a4a55));
        g.fillEllipse(r.getX() + 8.0f, r.getCentreY() - d * 0.5f, d, d);

        g.setColour(on ? juce::Colours::white : rlcol::textDim);
        g.setFont(makeFont(juce::jmin(13.0f, r.getHeight() * 0.55f), true));
        g.drawText(b.getButtonText(), r.withTrimmedLeft(18.0f), juce::Justification::centred, false);
    }

    void drawButtonBackground(juce::Graphics& g, juce::Button& b, const juce::Colour&, bool highlighted, bool down) override
    {
        auto r = b.getLocalBounds().toFloat().reduced(1.0f);
        g.setColour(down ? rlcol::accent : (highlighted ? juce::Colour(0xff2c2c36) : rlcol::knobBody));
        g.fillRoundedRectangle(r, 5.0f);
        g.setColour(highlighted ? rlcol::accent : rlcol::panelEdge);
        g.drawRoundedRectangle(r, 5.0f, 1.2f);
    }

    void drawComboBox(juce::Graphics& g, int width, int height, bool, int, int, int, int, juce::ComboBox& box) override
    {
        auto r = juce::Rectangle<float>(0.0f, 0.0f, (float) width, (float) height).reduced(0.5f);
        g.setColour(rlcol::knobBody);
        g.fillRoundedRectangle(r, 5.0f);
        g.setColour(box.hasKeyboardFocus(false) ? rlcol::accent : rlcol::panelEdge);
        g.drawRoundedRectangle(r, 5.0f, 1.2f);

        juce::Path arrow;
        const float cx = (float) width - 14.0f, cy = (float) height * 0.5f;
        arrow.addTriangle(cx - 4.0f, cy - 2.0f, cx + 4.0f, cy - 2.0f, cx, cy + 3.0f);
        g.setColour(rlcol::accent);
        g.fillPath(arrow);
    }

    void positionComboBoxText(juce::ComboBox& box, juce::Label& label) override
    {
        label.setBounds(8, 1, box.getWidth() - 28, box.getHeight() - 2);
        label.setFont(getComboBoxFont(box));
    }
};
