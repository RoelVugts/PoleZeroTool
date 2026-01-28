#include "State.h"

//==================================================================
const juce::Identifier State::IDs::type { "State" };
const juce::Identifier State::IDs::poleZeroState { "PoZeState" };

const juce::Identifier State::IDs::firstPlotType { "FirstPlotType" };
const juce::Identifier State::IDs::secondPlotType { "SecondPlotType" };

const juce::Identifier State::IDs::displayInDB { "DisplayInDB" };
const juce::Identifier State::IDs::displayLogarithmic { "DisplayLogarithmic" };
const juce::Identifier State::IDs::displayInHz { "DisplayInhZ" };

const juce::Identifier State::IDs::magnitudePlotRange { "MagPlotRange" };
const juce::Identifier State::IDs::phasePlotRange { "PhasePlotRange" };
const juce::Identifier State::IDs::groupDelayPlotRange { "GroupDelayPlotRange" };
const juce::Identifier State::IDs::phaseDelayPlotRange { "PhaseDelayPlotRange" };

State::State (juce::ValueTree tree_)
    : TreeWrapper(tree_)
    , poleZeroState (tree_.getOrCreateChildWithName (IDs::poleZeroState, nullptr))
    , firstPlotType (tree_, IDs::firstPlotType, PlotType::Magnitude)
    , secondPlotType (tree_, IDs::secondPlotType, PlotType::Phase)
    , displayInDB (tree_, IDs::displayInDB, true)
    , displayLogarithmic (tree_, IDs::displayLogarithmic, true)
    , displayInHz (tree_, IDs::displayInHz, true)
    , magnitudePlotRange (tree_, IDs::magnitudePlotRange, { juce::Decibels::decibelsToGain (-12.0f), juce::Decibels::decibelsToGain (12.0f) })
    , phasePlotRange (tree_, IDs::phasePlotRange, { -juce::MathConstants<float>::twoPi, juce::MathConstants<float>::twoPi })
    , groupDelayPlotRange (tree_, IDs::groupDelayPlotRange, { -4, 4 })
    , phaseDelayPlotRange (tree_, IDs::phaseDelayPlotRange, { -4, 4 })
{

}

void State::setState (const State& other)
{
    poleZeroState.setState (other.poleZeroState);
    firstPlotType.setValue (other.firstPlotType.getValue());
    secondPlotType.setValue (other.secondPlotType.getValue());

    displayInDB.setValue (other.displayInDB.getValue());
    displayLogarithmic.setValue (other.displayLogarithmic.getValue());
    displayInHz.setValue (other.displayInHz.getValue());

    magnitudePlotRange.setValue (other.magnitudePlotRange.getValue());
    phasePlotRange.setValue (other.phasePlotRange.getValue());
    groupDelayPlotRange.setValue (other.groupDelayPlotRange.getValue());
    phaseDelayPlotRange.setValue (other.phaseDelayPlotRange.getValue());
}

