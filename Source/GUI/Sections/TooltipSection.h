#pragma once

#include "../LookAndFeel.h"

#include <JuceHeader.h>

class ToolTipSection : public juce::Component, public juce::SettableTooltipClient
{
public:
    ToolTipSection()
    {
        SettableTooltipClient::setTooltip ("Displays help text");
        juce::Desktop::getInstance().addGlobalMouseListener (this);
    }

    ~ToolTipSection() override
    {
        juce::Desktop::getInstance().removeGlobalMouseListener (this);
    }

    void paint (Graphics& g) override
    {
        g.fillAll (LAF::Colours::darkBackgroundColour);

        auto bounds = getLocalBounds().toFloat();
        bounds.removeFromLeft (LAF::Layout::defaultSpacing);

        const float tipsTextWidth = juce::GlyphArrangement::getStringWidth (juce::FontOptions(12.0f), "Tips");
        auto tipsTextArea = bounds.removeFromLeft (tipsTextWidth * 2.0f);

        g.setFont (juce::FontOptions(12.0f));
        g.setColour (LAF::Colours::highlightedColour);
        g.drawFittedText ("Tips: ", tipsTextArea.toNearestInt(), juce::Justification::centred, 1, 0.9f);
        g.setColour (LAF::Colours::disabledTextColour);


        const float stringWidth = juce::GlyphArrangement::getStringWidth (juce::FontOptions(12.0f), toolTipText);
        if (stringWidth > bounds.getWidth())
            g.setFont (juce::FontOptions(11.0f));

        g.drawFittedText (toolTipText, bounds.toNearestInt(), juce::Justification::centredLeft, 1, 0.9f);
    }

private:
    void mouseEnter (const MouseEvent& event) override
    {
        if (auto* comp = dynamic_cast<juce::TooltipClient*> (event.eventComponent))
            toolTipText = comp->getTooltip();

        repaint();
    }

    void mouseExit(const MouseEvent& event) override
    {
        toolTipText = "";

        repaint();
    }

    juce::String toolTipText;
};