#pragma once

#include "../../DSP/FilterDesign.h"
#include "../PoleZeroState.h"

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
    // Returns true if the filter coefficients have changed
    bool filterChanged() const
    {
        return newCoefsReady.load(std::memory_order_acquire);
    }

    // Gets the most recently updated coefficients
    FilterDesign::CoefficientSet getCoefficients() const
    {
        const int readBuffer = activeBuffer.load(std::memory_order_acquire);
        return coefficients[readBuffer];
    }

    // Marks the coefficients as consumed.
    void markCoefficientsAsConsumed()
    {
        newCoefsReady.store(false, std::memory_order_release);
    }

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

        // Write to the inactive buffer
        const int writeBuffer = 1 - activeBuffer.load(std::memory_order_acquire);

        filterDesigner.setPoleZeros (poles, zeros);
        coefficients[writeBuffer].iirCoefs = filterDesigner.getIIRCoefs();
        coefficients[writeBuffer].firCoefs  = filterDesigner.getFIRCoefs();

        // Swap buffers
        activeBuffer.store(writeBuffer, std::memory_order_release);
        newCoefsReady.store (true, std::memory_order_release);
    }

    void filterGainChanged(FilterDesign* emitter) override
    {
        juce::MessageManager::callAsync ([this, emitter]() {
            gainAttachment.setValueAsCompleteGesture ((float)emitter->getGain());
        });

        // Write to the inactive buffer
        const int writeBuffer = 1 - activeBuffer.load(std::memory_order_acquire);
        coefficients[writeBuffer].iirCoefs = filterDesigner.getIIRCoefs();
        coefficients[writeBuffer].firCoefs  = filterDesigner.getFIRCoefs();

        // Swap buffers
        activeBuffer.store(writeBuffer, std::memory_order_release);
        newCoefsReady.store (true, std::memory_order_release);
    }

    //=========================================================================
    PoleZeroState state;
    FilterDesign& filterDesigner;

    //=========================================================================
    // 2 for double buffering
    FilterDesign::CoefficientSet coefficients[2];
    std::atomic<int> activeBuffer { 0 };
    std::atomic<bool> newCoefsReady { false };

    //=========================================================================
    juce::ParameterAttachment gainAttachment;

};