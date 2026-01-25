#pragma once

#include <JuceHeader.h>

#define APVTS_ID "APVTS"

enum PoZeParamID
{
    gain,
    autoNormalise,
    bypass,

    // Keep last to get total parameter count
    parameterCount
};

struct ParamMessage
{
    PoZeParamID id;
    float value;
};

const std::array<juce::String, parameterCount> paramID =
{
    "GAIN",
    "AUTO_NORMALISE"
       "BYPASS"
};

const std::array<juce::String, parameterCount> paramName
{
    "Gain"
       "Auto Normalise"
       "Bypass"
};