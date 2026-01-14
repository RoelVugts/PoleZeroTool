#pragma once

#include "Components/PoZePlot.h"
#include <JuceHeader.h>

namespace LAF
{
    namespace Colours
    {
        static const juce::Colour darkBackgroundColour { 25, 25, 25 };
    }
}

class PozeToolLaf : public juce::LookAndFeel_V4, public PoZePlot::Point::LookAndFeelMethods
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