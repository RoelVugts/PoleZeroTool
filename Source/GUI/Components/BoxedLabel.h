#pragma once

#include <JuceHeader.h>

class BoxedLabel : public juce::Component, public juce::SettableTooltipClient
{
public:

    enum ColourIds
    {
        backgroundColourId      = 0x200301,
        outlineColourId         = 0x200302,
    };

    BoxedLabel()
    {
        setColour (backgroundColourId, juce::Colours::black);
        setColour (outlineColourId, juce::Colours::grey);
        viewport.getHorizontalScrollBar().setColour (juce::ScrollBar::ColourIds::thumbColourId, juce::Colours::grey);
        viewport.setScrollBarThickness (5);
        viewport.setScrollBarsShown (false, true, false, true);

        label.setInterceptsMouseClicks (false, false);
        label.setJustificationType (juce::Justification::centredLeft);
        viewport.setViewedComponent (&label, false);
        addAndMakeVisible (viewport);
    }

    void paint(Graphics& g) override
    {
        auto bounds = getLocalBounds();
        const int scrollBarHeight = viewport.getScrollBarThickness();
        bounds.removeFromBottom (scrollBarHeight);

        g.setColour (findColour (backgroundColourId));
        g.fillRect(bounds);
        g.setColour (findColour (outlineColourId));
        g.drawRect (bounds);
    }

    void resized() override
    {
        auto bounds = getLocalBounds();
        bounds.reduce(5, 0);
        viewport.setBounds (bounds);

        const float fontHeight = (float)getHeight() * 0.23f;
        label.setFont (juce::FontOptions(fontHeight));

        bounds = getLocalBounds().reduced(padding);
        int maxWidth = 0;

        for (const auto& line : juce::StringArray::fromLines (label.getText ()))
        {
            const int lineWidth = juce::GlyphArrangement::getStringWidthInt (getLookAndFeel().getLabelFont (label), line);
            maxWidth = std::max(lineWidth, maxWidth);
        }

        bounds.expand (0, 3);
        label.setBounds (bounds.withWidth (maxWidth + 20));
    }

    void setText(const juce::String& text)
    {
        label.setText (text, juce::dontSendNotification);
        resized();
    }

    void setFont(const juce::Font& font)
    {
        label.setFont (font);
    }

private:

    juce::Label label;
    juce::Viewport viewport;

    static constexpr int padding { 5 };
};