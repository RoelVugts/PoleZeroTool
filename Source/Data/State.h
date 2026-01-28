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

        static const juce::Identifier firstPlotType;
        static const juce::Identifier secondPlotType;

        static const juce::Identifier displayInDB;
        static const juce::Identifier displayLogarithmic;
        static const juce::Identifier displayInHz;

        static const juce::Identifier magnitudePlotRange;
        static const juce::Identifier phasePlotRange;
        static const juce::Identifier groupDelayPlotRange;
        static const juce::Identifier phaseDelayPlotRange;
    };

    State(juce::ValueTree tree);

    void setState(const State& other);

    PoleZeroState poleZeroState;

    TreePropertyWrapper<PlotType> firstPlotType;
    TreePropertyWrapper<PlotType> secondPlotType;

    TreePropertyWrapper<bool> displayInDB;
    TreePropertyWrapper<bool> displayLogarithmic;
    TreePropertyWrapper<bool> displayInHz;

    TreePropertyWrapper<juce::Range<float>> magnitudePlotRange;
    TreePropertyWrapper<juce::Range<float>> phasePlotRange;
    TreePropertyWrapper<juce::Range<float>> groupDelayPlotRange;
    TreePropertyWrapper<juce::Range<float>> phaseDelayPlotRange;
};