#pragma once

#include "../../DSP/FilterDesign.h"
#include "../../GUI/Components/Plot.h"

#include <JuceHeader.h>

class ResponsePlotAttachment : private FilterDesign::Listener, private juce::ComponentListener, private juce::AsyncUpdater
{
public:

    ResponsePlotAttachment(State& settings, FilterDesign& filterDesign, Plot& magPlot, Plot& phasePlot_, Plot& groupDelayPlot_)
        : state(settings), filterDesigner (filterDesign), magnitudePlot (magPlot), phasePlot (phasePlot_), groupDelayPlot (groupDelayPlot_)
    {

        magnitudePlot.getDataFn = [this](float x) -> float {
            const int index = (int)(magnitudePlot.getXRange().convertTo0to1 (x) * (float)magnitudePlot.getNumDataPoints());
            jassert(index < cachedResponse.size());

            if (state.displayInDB.getValue())
                return (float)juce::Decibels::gainToDecibels (cachedResponse[index].magnitude);

            return (float)cachedResponse[index].magnitude;
        };

        phasePlot.getDataFn = [this](float x) -> float {
            int index = (int)(phasePlot.getXRange().convertTo0to1 (x) * (float)phasePlot.getNumDataPoints());
            jassert(index < cachedResponse.size());

            // 0 Hz has no phase so display phase of the next frequency
            if (index == 0)
                return (float)cachedResponse[1].phase;

            return (float)cachedResponse[index].phase;
        };

        groupDelayPlot.getDataFn = [this](float x) -> float {
            int index = (int)(groupDelayPlot.getXRange().convertTo0to1 (x) * (float)groupDelayPlot.getNumDataPoints());
            jassert(index < cachedResponse.size());

            if (index == 0)
                index += 1;
            else if (index == (int)cachedResponse.size() - 1)
                index -= 1;

            return (float)filterDesigner.getGroupDelay (cachedResponse[index], cachedResponse[index + 1]);
        };

        for (auto* plot : juce::Array<Plot*>{&magnitudePlot, &phasePlot, &groupDelayPlot})
            plot->dataRefreshFn = [this](int numDataPoints) {
                if (numDataPoints != cachedResponse.size())
                    updateResponse();
            };

        
        updateResponse();
        filterDesigner.addListener (this);
    }

    ~ResponsePlotAttachment() override
    {
        filterDesigner.removeListener (this);
    }

    void updateResponse()
    {
        const int numSamples = std::max(magnitudePlot.getNumDataPoints(), phasePlot.getNumDataPoints());
        const auto& range = state.displayLogarithmic.getValue() ? xRange = MappedRange<float>::createExponentialRange (0.0f, juce::MathConstants<float>::pi)
                                                                : xRange = MappedRange<float> { 0.0f, juce::MathConstants<float>::pi };

        cachedResponse.resize (numSamples);

        for (int i = 0; i < numSamples; i++)
        {
            const float angle = range.convertFrom0to1 ((float)i / (float)numSamples);
            cachedResponse[i] = filterDesigner.getFreqResponse (angle);
        }
    }

private:

    void filterCoefficientsChanged(FilterDesign*) override
    {
        triggerAsyncUpdate();
    }

    void filterGainChanged(FilterDesign*) override
    {
        triggerAsyncUpdate();
    }

    void updateAllPlots()
    {
        magnitudePlot.updatePath();
        phasePlot.updatePath();
        groupDelayPlot.updatePath();
    }

    void handleAsyncUpdate() override
    {
        updateResponse();
        updateAllPlots();
    }


    State state;
    Plot& magnitudePlot;
    Plot& phasePlot;
    Plot& groupDelayPlot;
    FilterDesign& filterDesigner;
    std::vector<FilterDesign::Response> cachedResponse;
    MappedRange<float> xRange;
};