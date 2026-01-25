#pragma once

#include <JuceHeader.h>

#include "Utils/TreeWrapper.h"

#include "PoleZeroState.h"

class State : public TreeWrapper
{
public:

    struct IDs
    {
        static const juce::Identifier type;
        static const juce::Identifier poleZeroState;
        static const juce::Identifier displayInDB;
        static const juce::Identifier displayGroupDelay;
        static const juce::Identifier displayLogarithmic;
        static const juce::Identifier displayInHz;
    };

    State(juce::ValueTree tree);

    void setState(const State& other);

    PoleZeroState poleZeroState;
    TreePropertyWrapper<bool> displayInDB;
    TreePropertyWrapper<bool> displayGroupDelay;
    TreePropertyWrapper<bool> displayLogarithmic;
    TreePropertyWrapper<bool> displayInHz;
};