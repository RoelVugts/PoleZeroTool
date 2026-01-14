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

    };

    State(juce::ValueTree tree);

    void setState(const State& other);

    PoleZeroState poleZeroState;
};