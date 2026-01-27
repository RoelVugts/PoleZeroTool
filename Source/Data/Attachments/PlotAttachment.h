#pragma once

#include "../../GUI/Components/Plot.h"
#include "../Utils/TreePropertyWrapper.h"
#include "DragBoxAttachment.h"
#include "PropertyAttachment.h"

class PlotAttachment : private Plot::Listener
{
public:
    PlotAttachment(Plot& plot, TreePropertyWrapper<juce::Range<float>> state)
        : responsePlot (plot), rangeAttachment (state, [this](const juce::Range<float>& range) {
            setRange (range);
        })
    {
        rangeAttachment.sendInitialUpdate();
        responsePlot.addListener (this);
    }

    ~PlotAttachment() override
    {
        responsePlot.removeListener (this);
    }

private:

    void setRange(const juce::Range<float>& newRange)
    {
        responsePlot.setRange ({ newRange.getStart(), newRange.getEnd() }, true);
    }

    void rangeChanged(Plot*) override
    {
        rangeAttachment.setPropertyValue ({ responsePlot.getYRange().start, responsePlot.getYRange().end } );
    }

    Plot& responsePlot;
    PropertyAttachment<juce::Range<float>> rangeAttachment;
};