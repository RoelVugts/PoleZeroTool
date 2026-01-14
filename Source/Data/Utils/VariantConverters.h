#pragma once

#include <JuceHeader.h>

#include "../GUI/Components/PoZePlot.h"
#include <complex>

template<>
struct juce::VariantConverter<std::complex<float>>
{
    static std::complex<float> fromVar(const juce::var& var)
    {
        jassert(var.isArray());

        const float real = var[0];
        const float imag = var[1];
        return std::complex<float>(real, imag);
    }

    static juce::var toVar(const std::complex<float>& complex)
    {
        juce::var var;
        var.append (complex.real());
        var.append (complex.imag());
        return var;
    }
};

template<>
struct juce::VariantConverter<PoZePlot::Point::Type>
{
    static PoZePlot::Point::Type fromVar(const juce::var& var)
    {
        jassert(var.isArray());

        int intVal = var;
        return static_cast<PoZePlot::Point::Type> (intVal);
    }

    static juce::var toVar(const PoZePlot::Point::Type& type)
    {
        return { static_cast<int> (type) };
    }
};