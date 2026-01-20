#pragma once

#include "../../DSP/FilterDesign.h"
#include "../../GUI/Components/ResponsePlot.h"

#include <JuceHeader.h>

class ResponsePlotAttachment : private FilterDesign::Listener
{
public:

    ResponsePlotAttachment(ResponsePlot& plotComp, FilterDesign& filterDesign, bool magIsTruePhaseIsFalse)
        : plot(plotComp), filterDesigner (filterDesign)
    {
        if (magIsTruePhaseIsFalse)
        {
            plot.getDataFn = [this](float angle) -> float {
                return filterDesigner.getFreqResponse (angle).magnitude;
            };
        }
        else
        {
            plot.getDataFn = [this](float angle) -> float {
                return filterDesigner.getFreqResponse (angle).phase;
            };
        }

        filterDesigner.addListener (this);
    }

    ~ResponsePlotAttachment() override
    {
        filterDesigner.removeListener (this);
    }

private:

    void filterCoefficientsChanged(FilterDesign*) override
    {
        plot.updatePath();
    }

    ResponsePlot& plot;
    FilterDesign& filterDesigner;
};