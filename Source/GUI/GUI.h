#pragma once

#include <JuceHeader.h>

#include "../Data/Attachments/PoZePlotAttachment.h"
#include "Components/PoZePlot.h"
#include "LookAndFeel.h"

class GUI : public juce::Component
{
public:

    GUI(AudioPluginAudioProcessor& p)
        : state(p.state)
    {
        poZePlot.setColour (PoZePlot::ColourIds::backgroundColourId, LAF::Colours::darkBackgroundColour);
        poZePlotAttachment = std::make_unique<PoZePlotAttachment>(state.poleZeroState, poZePlot, &p.undoManager);
        addAndMakeVisible (poZePlot);
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
        poZePlot.setBounds (xyPadArea.toNearestInt());
    }

private:
    State state;
    PoZePlot poZePlot;
    std::unique_ptr<PoZePlotAttachment> poZePlotAttachment;


};