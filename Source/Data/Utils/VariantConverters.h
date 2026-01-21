#pragma once

#include <JuceHeader.h>

#include "../GUI/Components/PoZePlot.h"
#include <complex>
#include <iso646.h>

template<>
struct juce::VariantConverter<PoZePlot::Point::Type>
{
    static PoZePlot::Point::Type fromVar(const juce::var& var)
    {
        int intVal = var;
        return static_cast<PoZePlot::Point::Type> (intVal);
    }

    static juce::var toVar(const PoZePlot::Point::Type& type)
    {
        return { static_cast<int> (type) };
    }
};