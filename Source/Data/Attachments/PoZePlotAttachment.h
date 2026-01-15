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

    PointAttachment(TreePropertyWrapper<std::complex<float>>& prop, PoZePlot::Point& pointComp)
        : point(pointComp)
        , attachment (prop, [this](const std::complex<float>& v) { setPointValue (v); })
    {
        attachment.sendInitialUpdate();

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

    void pointValueChanged(PoZePlot::Point* emitter) override
    {
        if (! ignoreCallbacks)
            attachment.setPropertyValue ({ emitter->getXValue(), emitter->getYValue() });
    }

    PoZePlot::Point& point;
    PropertyAttachment<std::complex<float>> attachment;
    bool ignoreCallbacks { false };
};

class PoZePlotAttachment : private PoZePlot::Listener
{
public:

    PoZePlotAttachment (const PoleZeroState& settings, PoZePlot& poZePlot, juce::UndoManager* um)
        : state(settings)
        , plot(poZePlot)
    {
        state.points.setOnChildAdded ([this] (const juce::ValueTree& addedChild) {
            PointState point(addedChild);
            std::complex<float> value = point.value.getValue();
            const int index = state.points.indexOf (addedChild);
            juce::ScopedValueSetter<bool> svs (ignoreCallbacks, true);

            if (! ignoreCallbacks)
                plot.addPoint (point.pointType.getValue(), value.real(), value.imag(), true);

            pointAttachments.add(std::make_unique<PointAttachment>(point.value, *plot.getPoint (index)));
        });

        state.points.setOnChildRemoved ([this] (const juce::ValueTree&, int index) {
            juce::ScopedValueSetter<bool> svs (ignoreCallbacks, true);

            if (! ignoreCallbacks)
                plot.removePoint (index, true);

            pointAttachments.remove (index);
        });

        state.points.setOnChildOrderChanged ([this](int, int) {
            rebuild();
        });

        rebuild();
        plot.addListener (this);
    }

    ~PoZePlotAttachment() override
    {
        plot.removeListener (this);
    }

private:

    void rebuild()
    {
        juce::ScopedValueSetter<bool> svs(ignoreCallbacks, true);
        plot.removeAllPoints (false);
        pointAttachments.clear ();

        const int numOldPoints = plot.getNumPoints();
        for (int i = 0; i < state.points.size(); i++)
        {
            PointState& pointState = state.points.getReference (i);
            std::complex<float> value = pointState.value.getValue();

            const bool sendNotification = i >= numOldPoints;
            auto* newPoint = plot.addPoint (pointState.pointType.getValue(), value.real(), value.imag(), sendNotification);
            pointAttachments.add(std::make_unique<PointAttachment>(pointState.value, *newPoint));
        }
    }

    void pointAdded(PoZePlot* emitter, int indexOfAddedPoint) override
    {
        if (! ignoreCallbacks)
        {
            auto* point = emitter->getPoint (indexOfAddedPoint);

            // Create new child
            auto pointState = std::make_unique<PointState>(juce::ValueTree(PointState::IDs::type));
            pointState->pointType.setValue (point->getType());
            pointState->value.setValue ({ point->getXValue(), point->getYValue() });

            state.points.add(std::move(pointState));
        }
    }

    void pointRemoved(PoZePlot* emitter, int indexOfRemovedPoint) override
    {
        if (! ignoreCallbacks)
            state.points.remove (indexOfRemovedPoint);
    }

    PoleZeroState state;
    PoZePlot& plot;
    juce::OwnedArray<PointAttachment> pointAttachments;
    bool ignoreCallbacks { false };

};