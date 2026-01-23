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

    };

    State(juce::ValueTree tree);

    void setState(const State& other);

    PoleZeroState poleZeroState;
    TreePropertyWrapper<bool> displayInDB;
};