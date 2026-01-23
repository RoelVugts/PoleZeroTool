#pragma once

#include "../../DSP/FilterDesign.h"
#include "../../GUI/Components/Plot.h"

#include <JuceHeader.h>

class ResponsePlotAttachment : private FilterDesign::Listener
{
public:

    ResponsePlotAttachment(State& settings, FilterDesign& filterDesign, Plot& magPlot, Plot& phasePlot_, Plot& groupDelayPlot_)
        : state(settings), filterDesigner (filterDesign), magnitudePlot (magPlot), phasePlot (phasePlot_), groupDelayPlot (groupDelayPlot_)
    {

        magnitudePlot.getDataFn = [this](float x) -> float {
            const int index = (int)(magnitudePlot.getXRange().convertTo0to1 (x) * magnitudePlot.getNumDataPoints());
            if (index >= cachedResponse.size())
                updateResponse();

            if (state.displayInDB.getValue())
                return (float)juce::Decibels::gainToDecibels (cachedResponse[index].magnitude);

            return cachedResponse[index].magnitude;
        };

        phasePlot.getDataFn = [this](float x) -> float {
            int index = (int)(phasePlot.getXRange().convertTo0to1 (x) * phasePlot.getNumDataPoints());
            if (index >= cachedResponse.size())
                updateResponse();

            return (float)cachedResponse[index].phase;
        };

        groupDelayPlot.getDataFn = [this](float x) -> float {
            int index = (int)(phasePlot.getXRange().convertTo0to1 (x) * phasePlot.getNumDataPoints());
            if (index >= cachedResponse.size())
                updateResponse();

            const float deltaX = phasePlot.getXRange().convertFrom0to1 (1.0f / (float)phasePlot.getNumDataPoints());
            if (index == 0)
                index += 1;
            else if (index == (int)cachedResponse.size() - 1)
                index -= 1;

            return (float)((cachedResponse[index + 1].phase - cachedResponse[index].phase) / deltaX);
        };


        updateResponse();
        filterDesigner.addListener (this);
    }

    ~ResponsePlotAttachment() override
    {
        filterDesigner.removeListener (this);
    }

private:

    void filterCoefficientsChanged(FilterDesign*) override
    {
        updatePlots();
    }

    void filterGainChanged(FilterDesign* emitter) override
    {
        updatePlots();
    }

    void updatePlots()
    {
        updateResponse();
        magnitudePlot.updatePath();
        phasePlot.updatePath();
    }

    void updateResponse()
    {
        const int numSamples = std::max(magnitudePlot.getNumDataPoints(), phasePlot.getNumDataPoints());

        cachedResponse.resize (numSamples);

        for (int i = 0; i < numSamples; i++)
        {
            const float angle = (float)i / (float)numSamples;
            cachedResponse[i] = filterDesigner.getFreqResponse (angle * juce::MathConstants<float>::pi);
        }
    }

    State state;
    Plot& magnitudePlot;
    Plot& phasePlot;
    Plot& groupDelayPlot;
    FilterDesign& filterDesigner;
    std::vector<FilterDesign::Response> cachedResponse;
};