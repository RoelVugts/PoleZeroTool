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

    PointAttachment(TreePropertyWrapper<std::complex<float>>& prop, PoZePlot::Point& pointComp, juce::UndoManager* um)
        : point(pointComp)
        , attachment (prop, [this](const std::complex<float>& v) { setPointValue (v); })
    {
        attachment.sendInitialUpdate();
    }

private:

    void setPointValue(const std::complex<float>& value)
    {
        juce::ScopedValueSetter<bool> svs (ignoreCallbacks, true);
        point.setNormalizedValue (value.real(), value.imag(), true);
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

            juce::ScopedValueSetter<bool> svs (ignoreCallbacks, true);
            auto* newPoint = plot.addPoint (point.isPole.getValue(), value.real(), value.imag(), true);
            pointAttachments.add(std::make_unique<PointAttachment>(point.value, *newPoint, nullptr));
        });

        state.points.setOnChildRemoved ([this] (const juce::ValueTree&, int index) {
            juce::ScopedValueSetter<bool> svs (ignoreCallbacks, true);

            pointAttachments.remove (index);
            plot.removePoint (index, true);
        });

        state.points.setOnChildOrderChanged ([this](int, int) {
            rebuild();
        });
    }

private:

    void rebuild()
    {
        juce::ScopedValueSetter<bool> svs(ignoreCallbacks, true);
        plot.removeAllPoints (true);
        pointAttachments.clear ();

        for (int i = 0; i < state.points.size(); i++)
        {
            PointState& pointState = state.points.getReference (i);
            std::complex<float> value = pointState.value.getValue();
            auto * newPoint = plot.addPoint (pointState.isPole.getValue(), value.real(), value.imag(), true);
            pointAttachments.add(std::make_unique<PointAttachment>(pointState.value, *newPoint, nullptr));
        }
    }

    void pointAdded(PoZePlot* emitter, int indexOfAddedPoint) override
    {
        if (! ignoreCallbacks)
            state.points.add(indexOfAddedPoint);
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