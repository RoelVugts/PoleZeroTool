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

    PointAttachment(TreePropertyWrapper<std::complex<float>>& valueProp, TreePropertyWrapper<PoZePlot::Point::Type>& typeProp, PoZePlot::Point& pointComp)
        : point(pointComp)
        , valueAttachment (valueProp, [this](const std::complex<float>& v) { setPointValue (v); })
        , typeAttachment (typeProp, [this](const PoZePlot::Point::Type& v) { setPointType (v); })
    {
        valueAttachment.sendInitialUpdate();

        point.addListener (this);
    }

    ~PointAttachment() override
    {
        point.removeListener (this);
    }

private:

    void setPointValue(const std::complex<float>& value)
    {
        juce::ScopedValueSetter<bool> svs (ignoreCallbacks, true);
        point.setValue (value.real(), value.imag(), true);
    }

    void setPointType(const PoZePlot::Point::Type& type)
    {
        juce::ScopedValueSetter<bool> svs (ignoreCallbacks, true);
        point.setType (type);
    }

    void pointValueChanged(PoZePlot::Point* emitter) override
    {
        if (! ignoreCallbacks)
            valueAttachment.setPropertyValue ({ emitter->getXValue(), emitter->getYValue() });
    }

    PoZePlot::Point& point;
    PropertyAttachment<std::complex<float>> valueAttachment;
    PropertyAttachment<PoZePlot::Point::Type> typeAttachment;
    bool ignoreCallbacks { false };
};

class PoZePlotAttachment : private PoZePlot::Listener, private juce::AsyncUpdater
{
public:

    PoZePlotAttachment (const PoleZeroState& settings, PoZePlot& poZePlot, juce::UndoManager* um)
        : state(settings)
        , plot(poZePlot)
    {
        state.points.setOnChildAdded ([this](juce::ValueTree&) {
            triggerAsyncUpdate();
        });

        state.points.setOnChildRemoved ([this](juce::ValueTree&, int) {
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

        plot.removeAllPoints (false);
        pointAttachments.clear ();

        const int numOldPoints = plot.getNumPoints();
        for (int i = 0; i < state.points.size(); i++)
        {
            PointState& pointState = state.points.getReference (i);
            std::complex<float> value = pointState.value.getValue();

            const bool sendNotification = i >= numOldPoints;
            auto* newPoint = plot.addPoint (pointState.pointType.getValue(), value.real(), value.imag(), sendNotification);
            pointAttachments.add(std::make_unique<PointAttachment>(pointState.value, pointState.pointType, *newPoint));
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
            pointState.value.setValue ({ point->getXValue(), point->getYValue() });

            const int conjugateIndex = point->isConjugate() ? plot.getPoints().indexOf (point->getConjugate()) : -1;
            pointState.conjugateIndex.setValue (conjugateIndex);
            pointAttachments.add(std::make_unique<PointAttachment>(pointState.value, pointState.pointType, *point));
        }

        state.points.setState (newState.points);
    }

    void pointAdded(PoZePlot* emitter, int indexOfAddedPoint) override
    {
        if (! ignoreGuiCallbacks)
            syncStateToPlot();

    }

    void pointRemoved(PoZePlot*, int indexOfRemovedPoint) override
    {
        if (! ignoreGuiCallbacks)
            syncStateToPlot();
    }

    void createdConjugatePair(PoZePlot* emitter, int index, int conjugateIndex) override
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