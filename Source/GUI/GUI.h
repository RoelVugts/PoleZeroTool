#pragma once

#include <JuceHeader.h>

#include "../Data/Attachments/PoZePlotAttachment.h"
#include "../Data/Attachments/ResponsePlotAttachment.h"
#include "Components/PoZePlot.h"
#include "Components/PoZeTable.h"
#include "Components/ResponsePlot.h"
#include "LookAndFeel.h"

class GUI : public juce::Component
{
public:

    GUI(AudioPluginAudioProcessor& p)
        : state(p.state)
        , poZeTable (p.state.poleZeroState)
    {
        poZePlot.setColour (PoZePlot::ColourIds::backgroundColourId, LAF::Colours::darkBackgroundColour);
        poZePlotAttachment = std::make_unique<PoZePlotAttachment>(state.poleZeroState, poZePlot, &p.undoManager);
        addAndMakeVisible (poZePlot);

        addAndMakeVisible (poZeTable);

        magnitudePlot.setRange ({ 0.0f, 100.0f });
        addAndMakeVisible (magnitudePlot);

        phasePlot.setRange ({ -12.54f, 12.54f });
        addAndMakeVisible (phasePlot);

        magAttachment = std::make_unique<ResponsePlotAttachment> (magnitudePlot, p.filterDesign, true);
        phaseAttachment = std::make_unique<ResponsePlotAttachment> (phasePlot, p.filterDesign, false);
    }

    void resized() override
    {
        auto bounds = getLocalBounds().toFloat();
        float width = bounds.getWidth();
        float height = bounds.getHeight();

        auto headerArea = bounds.removeFromTop (height * 0.05f);
        auto inputMeterArea = bounds.removeFromLeft (width * 0.1f);
        auto outputMeterArea = bounds.removeFromRight (width * 0.1f);

        auto leftArea = bounds.removeFromLeft (bounds.getWidth() * 0.5f);
        auto rightArea = bounds;

        const float xyPadSize = height * 0.45f;
        auto xyPadArea = leftArea.removeFromTop (xyPadSize).removeFromLeft (xyPadSize);
        poZePlot.setBounds (xyPadArea.toNearestInt());

        leftArea.removeFromTop (height * 0.02f);        // Spacing

        auto lowerArea = leftArea.removeFromTop (xyPadSize);
        auto tableArea = lowerArea.removeFromLeft (xyPadSize);
        poZeTable.setBounds (tableArea.toNearestInt());

        auto magnitudePlotArea = rightArea.removeFromTop (xyPadSize);
        magnitudePlot.setBounds (magnitudePlotArea.toNearestInt());

        rightArea.removeFromTop (height * 0.02f);        // Spacing

        auto phasePlotArea = rightArea.removeFromTop (xyPadSize);
        phasePlot.setBounds (phasePlotArea.toNearestInt());
    }

private:

    State state;
    PoZePlot poZePlot;
    std::unique_ptr<PoZePlotAttachment> poZePlotAttachment;

    PoZeTable poZeTable;
    ResponsePlot magnitudePlot;
    ResponsePlot phasePlot;
    std::unique_ptr<ResponsePlotAttachment> magAttachment, phaseAttachment;



};