#pragma once

#include "Components/PoZePlot.h"
#include "Components/DragBox.h"
#include <JuceHeader.h>

namespace LAF
{
    namespace Colours
    {
        static const juce::Colour primaryColour { 31, 38, 49 };
        static const juce::Colour secondaryColour { 25, 28, 36 };
        static const juce::Colour darkBackgroundColour { 21, 22, 28 };

        static const juce::Colour textColour { 227, 227, 227 };
        static const juce::Colour disabledTextColour { 179, 179, 179 };
        static const juce::Colour highlightedColour{ 228, 159, 125 };

        static const juce::Colour buttonColour{ 41, 46, 58 };
        static const juce::Colour buttonOnColour{ 72, 80, 101 };
        static const juce::Colour buttonOutlineColour{ 0, 0, 0 };

        static const juce::Colour plotBackgroundColour{ 1, 3, 7 };
        static const juce::Colour plotGridColour{ 153, 153, 153 };

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

    static void drawDragBoxBackground(juce::Graphics& g, DragBox& box)
    {
        auto bounds = box.getLocalBounds().toFloat();
        const float cornerSize = std::min(bounds.getHeight(), bounds.getWidth()) * 0.4f;

        g.setColour (box.findColour (DragBox::backgroundColourId));
        g.fillRoundedRectangle(bounds, cornerSize);

        static constexpr float lineThickness = 1.0f;
        bounds = bounds.reduced (lineThickness * 0.5f);

        g.setColour (box.findColour (DragBox::outlineColourId));
        g.drawRoundedRectangle (bounds, cornerSize, lineThickness);
    }
};