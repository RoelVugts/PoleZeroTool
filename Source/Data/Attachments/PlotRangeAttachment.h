#pragma once

#include "../../GUI/Components/Plot.h"
#include "../State.h"


class PlotRangeAttachment : private Plot::Listener
{
public:
    PlotRangeAttachment(State& settings, Plot& plot_, const int plotIndex)
        : state(settings), plot (plot_), index(plotIndex)
    {
        //======================================================================================================================
        state.setOnPropertyChanged (State::IDs::magnitudePlotRange, [this]() {
            if (getPlotType() == PlotType::Magnitude)
                setRange (state.magnitudePlotRange.getValue(), PlotType::Magnitude);
        }, true);

        state.setOnPropertyChanged (State::IDs::phasePlotRange, [this]() {
            if (getPlotType() == PlotType::Phase)
                setRange (state.phasePlotRange.getValue(), PlotType::Phase);
        }, true);

        state.setOnPropertyChanged (State::IDs::groupDelayPlotRange, [this]() {
            if (getPlotType() == PlotType::GroupDelay)
                setRange (state.groupDelayPlotRange.getValue(), PlotType::GroupDelay);
        }, true);

        state.setOnPropertyChanged (State::IDs::phaseDelayPlotRange, [this]() {
            if (getPlotType() == PlotType::PhaseDelay)
                setRange (state.phaseDelayPlotRange.getValue(), PlotType::PhaseDelay);
        }, true);

        //======================================================================================================================
        state.setOnPropertyChanged (State::IDs::displayInDB, [this]() {
            updateRange();
        }, true);

        //======================================================================================================================
        plot.addListener (this);
    }

    ~PlotRangeAttachment() override
    {
        plot.removeListener (this);
    }

    void updateRange()
    {
        const PlotType type = getPlotType();

        const auto range = [this, type]() {
            switch (type)
            {
                case PlotType::Magnitude: return state.magnitudePlotRange.getValue();
                case PlotType::Phase: return state.phasePlotRange.getValue();
                case PlotType::GroupDelay: return state.groupDelayPlotRange.getValue();
                case PlotType::PhaseDelay: return state.phaseDelayPlotRange.getValue();
            }

            jassertfalse;
            return state.magnitudePlotRange.getValue();
        }();

        if (type == PlotType::Magnitude)
        {
            const bool shouldDisplayInDecibels = state.displayInDB.getValue();

            if (shouldDisplayInDecibels)
                // Limit to -100 dB and +100 dB
                plot.setMinMaxRange (-100.0f, 100.0f);
            else
                // Limit gain plot minium to 0 (negative gain would be weird)
                plot.setMinMaxRange (0.0f, 100.0f);
        }
        else
        {
            // Set wide range for other plots
            plot.setMinMaxRange (-1000.0f, 1000.0f);
        }

        setRange (range, type);
    }

private:

    void setRange(const juce::Range<float>& newRange, PlotType type)
    {
        if (! ignoreCallbacks)
        {
            float start = newRange.getStart();
            float end = newRange.getEnd();

            if (type == PlotType::Magnitude)
            {
                // Convert values back to dB if we are in dB mode
                if (state.displayInDB.getValue())
                {
                    start = juce::Decibels::gainToDecibels (start);
                    end = juce::Decibels::gainToDecibels (end);
                }
            }

            plot.setRange ({ start, end }, true);
        }
    }

    void rangeChanged(Plot*) override
    {
        const PlotType type = getPlotType();
        const auto range = juce::Range<float>{ plot.getYRange().start, plot.getYRange().end };
        juce::ScopedValueSetter<bool> svs(ignoreCallbacks, true);

        switch (type)
        {
            case PlotType::Magnitude:
            {
                float start = plot.getYRange().start;
                float end = plot.getYRange().end;

                // Convert values back to gain if we are in dB mode
                if (state.displayInDB.getValue())
                {
                    start = juce::Decibels::decibelsToGain (start);
                    end = juce::Decibels::decibelsToGain (end);
                }

                state.magnitudePlotRange.setValue ({ start, end });
                break;
            }

            case PlotType::Phase:
                state.phasePlotRange.setValue (range);
                break;

            case PlotType::GroupDelay:
                state.groupDelayPlotRange.setValue (range);
                break;

            case PlotType::PhaseDelay:
                state.phaseDelayPlotRange.setValue (range);
                break;
        }
    }

    PlotType getPlotType() const
    {
        return index == 0 ? state.firstPlotType.getValue()
                          : state.secondPlotType.getValue();
    }

    State state;
    Plot& plot;
    const int index;
    bool ignoreCallbacks { false };
};