#pragma once

#include <JuceHeader.h>

#include "../Components/PoZePlot.h"
#include "../Components/PoZeTable.h"
#include "../../Data/Attachments/PoZePlotAttachment.h"
#include "../LookAndFeel.h"

class PoleZeroSection : public juce::Component
{
public:

    PoleZeroSection(AudioPluginAudioProcessor& p)
        : poZeTable (p.state.poleZeroState)
    {
        poZePlot.setColour (PoZePlot::ColourIds::backgroundColourId, LAF::Colours::plotBackgroundColour);
        poZePlot.setColour (PoZePlot::ColourIds::unitCircleColourId, LAF::Colours::plotGridColour);

        #if JUCE_MAC
            poZePlot.setTooltip ("The Pole Zero plot | Command+Click: Add Pole | Shift+Click: Add Zero | Alt+Click: Remove Pole/Zero | Command+Shift+Click: Add conjugate");
        #else
            poZePlot.setTooltip ("The Pole Zero plot | Command+Click: Add Pole | Shift+Click: Add Zero | Alt+Click: Remove Pole/Zero | Ctrl+Shift+Click: Add conjugate");
        #endif
        poZePlotAttachment = std::make_unique<PoZePlotAttachment>(p.state.poleZeroState, poZePlot, &p.undoManager);
        addAndMakeVisible (poZePlot);

        poZeTable.setTooltip ("Editable list of the Poles and Zeros");
        addAndMakeVisible (poZeTable);
    }

    void resized() override
    {
        auto bounds = getLocalBounds().toFloat();

        const float poZePlotSize = bounds.getWidth();
        auto plotArea = bounds.removeFromTop (poZePlotSize);
        poZePlot.setBounds (plotArea.toNearestInt());

        bounds.removeFromTop (LAF::Layout::defaultSpacing); // Spacing

        auto tableArea = bounds;
        poZeTable.setBounds (tableArea.toNearestInt());
    }

    PoZePlot poZePlot;
    std::unique_ptr<PoZePlotAttachment> poZePlotAttachment;

    PoZeTable poZeTable;
};