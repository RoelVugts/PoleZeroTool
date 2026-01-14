#pragma once

#include <JuceHeader.h>

#include "Components/PoZePlot.h"
#include "LookAndFeel.h"

class GUI : public juce::Component
{
public:

    GUI()
    {
        xyPad.setColour (PoZePlot::ColourIds::backgroundColourId, LAF::Colours::darkBackgroundColour);
        addAndMakeVisible (xyPad);
    }

    void resized() override
    {
        auto bounds = getLocalBounds().toFloat();
        const float width = bounds.getWidth();
        const float height = bounds.getHeight();

        auto headerArea = bounds.removeFromTop (height * 0.05f);
        auto inputMeterArea = bounds.removeFromLeft (width * 0.1f);

        const float xyPadSize = std::min(width, height) * 0.45f;
        auto xyPadArea = bounds.removeFromTop (xyPadSize).removeFromLeft (xyPadSize);
        xyPad.setBounds (xyPadArea.toNearestInt());
    }

    PoZePlot xyPad;
};