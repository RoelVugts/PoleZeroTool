#include "State.h"

//==================================================================
const juce::Identifier State::IDs::type { "State" };
const juce::Identifier State::IDs::poleZeroState { "PoZeState" };
const juce::Identifier State::IDs::displayInDB { "DisplayInDB" };
const juce::Identifier State::IDs::displayGroupDelay { "DisplayGroupDelay" };
const juce::Identifier State::IDs::displayLogarithmic { "DisplayLogarithmic" };
const juce::Identifier State::IDs::displayInHz { "DisplayInhZ" };

const juce::Identifier State::IDs::magnitudePlotRange { "MagPlotRange" };
const juce::Identifier State::IDs::magnitudePlotRangeDB { "MagPlotRangeDB" };
const juce::Identifier State::IDs::phasePlotRange { "PhasePlotRange" };
const juce::Identifier State::IDs::groupDelayPlotRange { "GroupDelayPlotRange" };

State::State (juce::ValueTree tree_)
    : TreeWrapper(tree_)
    , poleZeroState (tree_.getOrCreateChildWithName (IDs::poleZeroState, nullptr))
    , displayInDB (tree_, IDs::displayInDB, true)
    , displayGroupDelay (tree_, IDs::displayGroupDelay, false)
    , displayLogarithmic (tree_, IDs::displayLogarithmic, true)
    , displayInHz (tree_, IDs::displayInHz, true)
    , magnitudePlotRange (tree_, IDs::magnitudePlotRange, { juce::Decibels::decibelsToGain (-12.0f), juce::Decibels::decibelsToGain (12.0f) })
    , magnitudePlotRangeDB (tree_, IDs::magnitudePlotRangeDB, { -12.0f, 12.0f })
    , phasePlotRange (tree_, IDs::phasePlotRange, { -juce::MathConstants<float>::twoPi, juce::MathConstants<float>::twoPi })
    , groupDelayPlotRange (tree_, IDs::groupDelayPlotRange, { -4, 4 })
{

}

void State::setState (const State& other)
{
    poleZeroState.setState (other.poleZeroState);
    displayInDB.setValue (other.displayInDB.getValue());
    displayGroupDelay.setValue (other.displayGroupDelay.getValue());
    displayLogarithmic.setValue (other.displayLogarithmic.getValue());
    displayInHz.setValue (other.displayInHz.getValue());

    magnitudePlotRange.setValue (other.magnitudePlotRange.getValue());
    magnitudePlotRangeDB.setValue (other.magnitudePlotRangeDB.getValue());
    phasePlotRange.setValue (other.phasePlotRange.getValue());
    groupDelayPlotRange.setValue (other.groupDelayPlotRange.getValue());
}

