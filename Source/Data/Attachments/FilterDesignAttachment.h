#pragma once

#include "../../DSP/FilterDesign.h"
#include "../PoleZeroState.h"

#include <JuceHeader.h>

class FilterDesignAttachment : private juce::AsyncUpdater
{
public:

    FilterDesignAttachment(PoleZeroState settings, FilterDesign& designer)
        : state(settings), filterDesigner (designer)
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
    }

    bool filterChanged() const
    {
        return newCoefsReady.load(std::memory_order_acquire);
    }

    FilterDesign::CoefficientSet getCoefficients() const
    {
        const int readBuffer = activeBuffer.load(std::memory_order_acquire);
        return coefficients[readBuffer];
    }

    void markCoefficientsAsConsumed()
    {
        newCoefsReady.store(false, std::memory_order_release);
    }

private:

    void triggerUpdate()
    {
        if (isUpdatePending())
            cancelPendingUpdate();

        triggerAsyncUpdate();
    }

    void handleAsyncUpdate() override
    {
        std::vector<std::complex<float>> poles;
        std::vector<std::complex<float>> zeros;

        for (int i = 0; i < state.points.size(); i++)
        {
            const auto point = state.points.getReference (i);
            const auto type = point.pointType.getValue();
            switch (type)
            {
                case PoZePlot::Point::Type::pole:
                    poles.emplace_back (point.value.getValue());
                    break;

                case PoZePlot::Point::Type::zero:
                    zeros.emplace_back (point.value.getValue());
                    break;
            }
        }

        int writeBuffer = 1 - activeBuffer.load(std::memory_order_acquire);

        filterDesigner.setPoleZeros (poles, zeros);
        coefficients[writeBuffer].iirCoefs = filterDesigner.getIIRCoefs();
        coefficients[writeBuffer].firCoefs  = filterDesigner.getFIRCoefs();

        activeBuffer.store(writeBuffer, std::memory_order_release);
        newCoefsReady.store (true, std::memory_order_release);
    }

    PoleZeroState state;
    FilterDesign& filterDesigner;

    //=========================================================================
    // 2 for double buffering
    FilterDesign::CoefficientSet coefficients[2];
    std::atomic<int> activeBuffer { 0 };
    std::atomic<bool> newCoefsReady { false };
};