#pragma once

#include "../Utils/Utils.h"

#include "magic_enum/magic_enum.hpp"
#include <JuceHeader.h>

#define APVTS_ID "APVTS"
#define MINUS_INFINITY_DB -200.0

enum PoZeParamID
{
    Gain,
    AutoNormalise,
    Bypass,

    // Keep last to get total parameter count
    ParameterCount
};

enum class PlotType
{
    Magnitude,
    Phase,
    GroupDelay,
    PhaseDelay,
};

struct ParamMessage
{
    PoZeParamID id;
    float value;
};

inline juce::String getParamID(PoZeParamID id)
{
    const juce::String enumName = magic_enum::enum_name(id).data();
    return enumName.toUpperCase();
}

inline juce::String getParamName(PoZeParamID id)
{
    const juce::String enumName = magic_enum::enum_name(id).data();
    return Utils::insertSpacesForEveryUpperCase (enumName);
}

static constexpr double minPoZePlotRange { -1.5 };
static constexpr double maxPoZePlotRange { 1.5 };