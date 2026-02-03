#pragma once

#include <JuceHeader.h>

#include "../../Plugin/Parameters.h"
#include "../../GUI/Components/PoZePlot.h"

//==================================================
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

//==================================================
template <>
struct juce::VariantConverter<juce::Range<float>>
{
    static juce::Range<float> fromVar(const juce::var& var)
    {
        const uint64_t packed = static_cast<uint64_t> (static_cast<int64> (var));

        // Extract real and imaginary part
        uint32_t startBits = static_cast<uint32_t> (packed >> 32);
        uint32_t endBits = static_cast<uint32_t> (packed & 0xffffffffu);

        float start;
        float end;

        // Bits are stored in integers but represented as float, so copy directly to float
        std::memcpy(&start, &startBits, sizeof(uint32_t));
        std::memcpy(&end, &endBits, sizeof(uint32_t));

        return { start, end };
    }

    static juce::var toVar(const juce::Range<float>& range)
    {
        static_assert (sizeof (float) == 4);
        static_assert (sizeof (int32_t) == 4);

        uint32_t realBits;
        uint32_t imagBits;

        const float real = range.getStart();
        const float imag = range.getEnd();

        // Write to integers keeping float representation
        std::memcpy(&realBits, &real, sizeof(float));
        std::memcpy(&imagBits, &imag, sizeof(float));

        // Pack both int32 into one int64
        const int64 packed = static_cast<int64_t>(static_cast<uint64_t> (realBits) << 32 | static_cast<uint64_t> (imagBits));

        return { packed };
    }
};

//==================================================
template <>
struct juce::VariantConverter<PlotType>
{
    static PlotType fromVar(const juce::var& var)
    {
        const int enumValue = var;
        return static_cast<PlotType>(enumValue);
    }

    static juce::var toVar(const PlotType& type)
    {
        const int enumValue = static_cast<int>(type);
        return { enumValue };
    }
};