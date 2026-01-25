#pragma once

#include <JuceHeader.h>

#include "../../GUI/Components/PoZePlot.h"
#include "../PoleZeroState.h"
#include "PropertyAttachment.h"

#include "../../Gui/Components/PoZePlot.h"
#include <complex>

class PointAttachment : private PoZePlot::Point::Listener
{
public:

    PointAttachment(PointState settings, PoZePlot::Point& pointComp)
        : state(settings)
        , point(pointComp)
        , realAttachment (state.real, [this](const double& v) { setRealValue (v); })
        , imagAttachment (state.imag, [this](const double& v) { setImagValue (v); })
        , typeAttachment (state.pointType, [this](const PoZePlot::Point::Type& v) { setPointType (v); })
    {
        realAttachment.sendInitialUpdate();
        imagAttachment.sendInitialUpdate();
        typeAttachment.sendInitialUpdate();

        point.addListener (this);
    }

    ~PointAttachment() override
    {
        point.removeListener (this);
    }

private:

    void setRealValue(const double& value)
    {
        juce::ScopedValueSetter<bool> svs (ignoreCallbacks, true);
        point.setXValue (value, true);
    }

    void setImagValue(const double& value)
    {
        juce::ScopedValueSetter<bool> svs (ignoreCallbacks, true);
        point.setYValue (value, true);
    }

    void setPointType(const PoZePlot::Point::Type& type)
    {
        juce::ScopedValueSetter<bool> svs (ignoreCallbacks, true);
        point.setType (type);
    }

    void pointValueChanged(PoZePlot::Point* emitter) override
    {
        if (! ignoreCallbacks)
        {
            const double real = emitter->getXValue();
            double imag = emitter->getYValue();;

            realAttachment.setPropertyValue (real);
            imagAttachment.setPropertyValue (imag);
        }
    }

    PointState state;
    PoZePlot::Point& point;
    PropertyAttachment<double> realAttachment;
    PropertyAttachment<double> imagAttachment;
    PropertyAttachment<PoZePlot::Point::Type> typeAttachment;
    bool ignoreCallbacks { false };
};

class PoZePlotAttachment : private PoZePlot::Listener, private juce::AsyncUpdater
{
public:

    PoZePlotAttachment (const PoleZeroState& settings, PoZePlot& poZePlot, juce::UndoManager*)
        : state(settings)
        , plot(poZePlot)
    {
        state.points.setOnChildAdded ([this](juce::ValueTree&) {
            if (! ignoreStateCallbacks)
                triggerAsyncUpdate();
        });

        state.points.setOnChildRemoved ([this](juce::ValueTree&, int) {
            if (! ignoreStateCallbacks)
                triggerAsyncUpdate();
        });

        //=========================================================================
        syncPlotToState();
        plot.addListener (this);
    }

    ~PoZePlotAttachment() override
    {
        plot.removeListener (this);
    }

private:

    void syncPlotToState()
    {
        juce::ScopedValueSetter<bool> svs(ignoreGuiCallbacks, true);

        const int numOldPoints = plot.getNumPoints();
        plot.removeAllPoints (false);
        pointAttachments.clear ();

        for (int i = 0; i < state.points.size(); i++)
        {
            PointState& pointState = state.points.getReference (i);
            std::complex<double> value = { pointState.real.getValue(), pointState.imag.getValue() };

            const bool sendNotification = i >= numOldPoints;
            auto* newPoint = plot.addPoint (pointState.pointType.getValue(), (float)value.real(), (float)value.imag(), sendNotification);
            pointAttachments.add(std::make_unique<PointAttachment>(pointState, *newPoint));
        }

        // Set conjugates after all points are added
        for (int i = 0; i < state.points.size(); i++)
        {
            PointState& pointState = state.points.getReference (i);
            auto* point = plot.getPoint (i);
            if (const int idx = pointState.conjugateIndex.getValue(); idx >= 0)
                point->setConjugate (plot.getPoint (idx));
        }
    }

    void syncStateToPlot()
    {
        juce::ScopedValueSetter<bool> svs (ignoreStateCallbacks, true);

        PoleZeroState newState { juce::ValueTree(PoleZeroState::IDs::type) };
        pointAttachments.clear ();

        for (int i = 0; i < plot.getNumPoints(); i++)
        {
            auto* point = plot.getPoint (i);
            newState.points.add ();
            auto& pointState = newState.points.getReference (i);
            pointState.pointType.setValue (point->getType());

            pointState.real.setValue (point->getXValue());
            pointState.imag.setValue (point->getYValue());

            const int conjugateIndex = point->isConjugate() ? plot.getPoints().indexOf (point->getConjugate()) : -1;
            pointState.conjugateIndex.setValue (conjugateIndex);
            pointAttachments.add(std::make_unique<PointAttachment>(pointState, *point));
        }

        state.points.setState (newState.points);
    }

    void pointAdded(PoZePlot*, int) override
    {
        if (! ignoreGuiCallbacks)
            syncStateToPlot();
    }

    void pointRemoved(PoZePlot*, int) override
    {
        if (! ignoreGuiCallbacks)
            syncStateToPlot();
    }

    void createdConjugatePair(PoZePlot*, int, int) override
    {
        if (! ignoreGuiCallbacks)
            syncStateToPlot();
    }

    void handleAsyncUpdate() override
    {
        syncPlotToState();
    }

    PoleZeroState state;
    PoZePlot& plot;
    juce::OwnedArray<PointAttachment> pointAttachments;
    bool ignoreStateCallbacks { false };
    bool ignoreGuiCallbacks { false };

};