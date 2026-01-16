#pragma once

#include <JuceHeader.h>

#include "../Data/Attachments/PoZePlotAttachment.h"
#include "Components/Plot.h"
#include "Components/PoZePlot.h"
#include "Components/PoZeTable.h"
#include "LookAndFeel.h"

class GUI : public juce::Component
{
public:

    GUI(AudioPluginAudioProcessor& p)
        : state(p.state), poZeTable (p.state.poleZeroState)
    {
        poZePlot.setColour (PoZePlot::ColourIds::backgroundColourId, LAF::Colours::darkBackgroundColour);
        poZePlotAttachment = std::make_unique<PoZePlotAttachment>(state.poleZeroState, poZePlot, &p.undoManager);
        addAndMakeVisible (poZePlot);

        addAndMakeVisible (poZeTable);

        addAndMakeVisible (magnitudePlot);
    }

    void resized() override
    {
        auto bounds = getLocalBounds().toFloat();
        const float width = bounds.getWidth();
        const float height = bounds.getHeight();

        auto headerArea = bounds.removeFromTop (height * 0.05f);
        auto inputMeterArea = bounds.removeFromLeft (width * 0.1f);
        auto outputMeterArea = bounds.removeFromRight (width * 0.1f);

        const float xyPadSize = height * 0.45f;
        auto upperArea = bounds.removeFromTop (xyPadSize);
        auto xyPadArea = upperArea.removeFromLeft (xyPadSize);
        poZePlot.setBounds (xyPadArea.toNearestInt());

        upperArea.removeFromLeft (width * 0.02f);        // Spacing
        upperArea.removeFromRight (width * 0.02f);

        magnitudePlot.setBounds (upperArea.toNearestInt());

        bounds.removeFromTop (height * 0.02f);        // Spacing

        auto tableArea = bounds.removeFromTop (height * 0.4f).removeFromLeft (xyPadSize);
        poZeTable.setBounds (tableArea.toNearestInt());


    }

private:

    State state;
    PoZePlot poZePlot;
    std::unique_ptr<PoZePlotAttachment> poZePlotAttachment;

    PoZeTable poZeTable;
    ResponsePlot magnitudePlot;


};