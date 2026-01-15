#pragma once

#include <JuceHeader.h>

#include "../GUI/Components/PoZePlot.h"
#include <complex>
#include <iso646.h>

template <>
struct juce::VariantConverter<std::complex<float>>
{
    static std::complex<float> fromVar(const juce::var& var)
    {
        const uint64_t packed = static_cast<uint64_t> (static_cast<int64_t> (var));

        // Extract real and imaginary part
        uint32_t realBits = static_cast<uint32_t> (packed >> 32);
        uint32_t imagBits = static_cast<uint32_t> (packed & 0xffffffffu);

        float real;
        float imag;

        // Bits are stored in integers but represented as float, so copy directly to float
        std::memcpy(&real, &realBits, sizeof(uint32_t));
        std::memcpy(&imag, &imagBits, sizeof(uint32_t));

        return { real, imag };
    }

    static juce::var toVar(const std::complex<float>& complex)
    {
        static_assert (sizeof (float) == 4);
        static_assert (sizeof (int32_t) == 4);

        uint32_t realBits;
        uint32_t imagBits;

        const float real = complex.real();
        const float imag = complex.imag();

        // Write to integers keeping float representation
        std::memcpy(&realBits, &real, sizeof(float));
        std::memcpy(&imagBits, &imag, sizeof(float));

        // Pack both int32 into one int64
        const int64_t packed = static_cast<int64_t>(static_cast<uint64_t> (realBits) << 32 | static_cast<uint64_t> (imagBits));

        return { packed };
    }
};

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