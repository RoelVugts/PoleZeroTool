#pragma once

#include "../../DSP/FilterDesign.h"
#include "../PoleZeroState.h"
#include "../../Utils/TripleBuffer.h"

#include <JuceHeader.h>

/** Attachment to keep the filter designer and the state in sync.*/
class FilterDesignAttachment : private juce::AsyncUpdater, private FilterDesign::Listener
{
public:

    FilterDesignAttachment(PoleZeroState settings, FilterDesign& designer, juce::RangedAudioParameter& gainParam)
        : state(settings), filterDesigner (designer), gainAttachment (gainParam, [this](float v) { filterDesigner.setGain ((double)v); })
    {
        state.points.setOnChildAdded ([this](juce::ValueTree&) {
            triggerUpdate();
        });

        state.points.setOnChildRemoved ([this](juce::ValueTree&, int) {
            triggerUpdate();
        });

        state.setOnAnyPropertyChanged ([this](juce::ValueTree&) {
            triggerUpdate();
        });

        //=========================================================================
        FilterDesignAttachment::handleAsyncUpdate();

        filterDesigner.addListener (this);
    }

    ~FilterDesignAttachment() override
    {
        filterDesigner.removeListener (this);
    }

    //=========================================================================
    // Gets the most recently updated coefficients in a thread safe
    // and lock-free way
    FilterDesign::CoefficientSet getCoefficients()
    {
        return coefficients.read();
    }

    [[nodiscard]] bool newCoefsReady() const { return coefficients.newDataReady(); }

private:
    //=========================================================================
    void triggerUpdate()
    {
        if (isUpdatePending())
            cancelPendingUpdate();

        triggerAsyncUpdate();
    }

    void handleAsyncUpdate() override
    {
        std::vector<std::complex<double>> poles;
        std::vector<std::complex<double>> zeros;

        for (int i = 0; i < state.points.size(); i++)
        {
            const auto point = state.points.getReference (i);
            const auto type = point.pointType.getValue();
            switch (type)
            {
                case PoZePlot::Point::Type::pole:
                    poles.emplace_back (point.real.getValue(), point.imag.getValue());
                    break;

                case PoZePlot::Point::Type::zero:
                    zeros.emplace_back (point.real.getValue(), point.imag.getValue());
                    break;
            }
        }

        filterDesigner.setPoleZeros (poles, zeros);

        FilterDesign::CoefficientSet newCoefs = filterDesigner.getCoefficientSet();
        coefficients.write (newCoefs);
    }

    void filterGainChanged(FilterDesign* emitter) override
    {
        juce::MessageManager::callAsync ([this, emitter]() {
            gainAttachment.setValueAsCompleteGesture ((float)emitter->getGain());
        });

        FilterDesign::CoefficientSet newCoefs = filterDesigner.getCoefficientSet();
        coefficients.write (newCoefs);
    }

    //=========================================================================
    PoleZeroState state;
    FilterDesign& filterDesigner;

    //=========================================================================
    TripleBuffer<FilterDesign::CoefficientSet> coefficients;

    //=========================================================================
    juce::ParameterAttachment gainAttachment;

};