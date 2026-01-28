#pragma once

#include "../../DSP/FilterDesign.h"
#include "../../GUI/Components/Plot.h"

#include <JuceHeader.h>

class PlotDataAttachment : private FilterDesign::Listener, private juce::ComponentListener, private juce::AsyncUpdater, private Plot::Listener
{
public:

    PlotDataAttachment(State& settings, FilterDesign& filterDesign, Plot& fistPlot_, Plot& secondPlot_)
        : state(settings), filterDesigner (filterDesign), firstPlot (fistPlot_), secondPlot (secondPlot_)
    {
        //======================================================================================================================
        for (auto* plot : juce::Array<Plot*>{&firstPlot, &secondPlot})
            plot->onNumDataPointsChanged = [this](int numDataPoints) {
                // Update the cached response for the number of data points
                if (numDataPoints != cachedResponse.size())
                    updateResponse();
            };

        firstPlot.getDataFn = [this](float, int index) -> float {
            return getDataPointForPlotType (index, state.firstPlotType.getValue());
        };

        secondPlot.getDataFn = [this](float, int index) -> float {
            return getDataPointForPlotType (index, state.secondPlotType.getValue());
        };

        //======================================================================================================================
        // Initialize
        updateResponse();
        filterDesigner.addListener (this);
    }

    ~PlotDataAttachment() override
    {
        filterDesigner.removeListener (this);
    }

    void updateResponse()
    {
        const int numSamples = std::max(firstPlot.getNumDataPoints(), secondPlot.getNumDataPoints());
        const auto& range = state.displayLogarithmic.getValue() ? MappedRange<double>::createExponentialRange (0.0f, juce::MathConstants<double>::pi)
                                                                : MappedRange<double> { 0.0f, juce::MathConstants<double>::pi };

        cachedResponse.resize (numSamples);

        for (int i = 0; i < numSamples; i++)
        {
            const double angle = range.convertFrom0to1 ((double)i / (double)(numSamples - 1));
            cachedResponse[i] = filterDesigner.getFreqResponse (angle);
        }
    }

private:
    //======================================================================================================================
    // Plot update methods
    void updateAllPlots() const
    {
        firstPlot.updatePath();
        secondPlot.updatePath();
    }

    void handleAsyncUpdate() override
    {
        updateResponse();
        updateAllPlots();
    }

    //======================================================================================================================
    // FilterDesign listener callbacks
    void filterCoefficientsChanged(FilterDesign*) override
    {
        triggerAsyncUpdate();
    }

    void filterGainChanged(FilterDesign*) override
    {
        triggerAsyncUpdate();
    }

    //======================================================================================================================
    float getDataPointForPlotType(int index, PlotType type)
    {
        switch (type)
        {
            case PlotType::Magnitude:
            {
                jassert(index < cachedResponse.size());

                if (state.displayInDB.getValue())
                    return (float)juce::Decibels::gainToDecibels (cachedResponse[index].magnitude, MINUS_INFINITY_DB);

                return (float)cachedResponse[index].magnitude;
            }

            case PlotType::Phase:
            {
                jassert(index < cachedResponse.size());

                // 0 Hz has no phase so display phase of the next frequency
                if (index == 0)
                    return (float)cachedResponse[1].phase;

                return (float)cachedResponse[index].phase;
            }

            case PlotType::GroupDelay:
            {
                jassert(index < cachedResponse.size());

                if (index == 0)
                    index += 1;
                else if (index == (int)cachedResponse.size() - 1)
                    index -= 1;

                return (float)filterDesigner.getGroupDelay (cachedResponse[index], cachedResponse[index + 1]);
            }

            case PlotType::PhaseDelay:
            {

                // 0 Hz has no phase so display phase delay of the next frequency
                if (index == 0)
                    return (float)cachedResponse[1].phaseDelay;

                return (float)cachedResponse[index].phaseDelay;
            }

            default:
                jassertfalse; break;
        }

        // Should never reach here...
        jassertfalse;
        return 0.0f;
    }

    State state;
    Plot& firstPlot;
    Plot& secondPlot;
    FilterDesign& filterDesigner;
    std::vector<FilterDesign::Response> cachedResponse;
    MappedRange<float> xRange;

    bool ignoreCallbacks { false };
};