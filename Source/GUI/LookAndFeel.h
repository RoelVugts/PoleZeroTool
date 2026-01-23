#pragma once

#include "Components/PoZePlot.h"
#include <JuceHeader.h>

namespace LAF
{
    namespace Colours
    {
        static const juce::Colour darkBackgroundColour { 25, 25, 25 };
        static const juce::Colour textColour { 200, 200, 200 };
        static const juce::Colour buttonOffColour{ 20, 20, 20 };
        static const juce::Colour buttonOnColour{ 60, 60, 60 };
    }

    namespace Layout
    {
        static constexpr float defaultSpacing { 10.0f };
        static constexpr float poZeContentWidthRatio { 0.4f };
    }

}

class PoZeToolLaf : public juce::LookAndFeel_V4, public PoZePlot::Point::LookAndFeelMethods
{
public:

    void drawPoZePoint(juce::Graphics& g, PoZePlot::Point& p, bool mouseIsOverOrDragging) override
    {
        auto bounds = p.getLocalBounds().toFloat();
        g.setColour (mouseIsOverOrDragging ? juce::Colours::white : juce::Colours::grey);

        switch (p.getType())
        {
            case PoZePlot::Point::Type::pole:
            {
                g.drawLine(bounds.getX(), bounds.getY(), bounds.getRight(), bounds.getBottom());
                g.drawLine(bounds.getX(), bounds.getBottom(), bounds.getRight(), bounds.getY());
                break;
            }

            case PoZePlot::Point::Type::zero:
            {
                constexpr float lineThickness = 1.0f;
                g.drawEllipse (bounds, lineThickness);
                break;
            }
        }
    }
};