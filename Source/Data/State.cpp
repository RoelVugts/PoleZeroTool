#include "State.h"

const juce::Identifier State::IDs::type { "State" };
const juce::Identifier State::IDs::poleZeroState { "PoZeState" };
const juce::Identifier State::IDs::displayInDB { "DisplayInDB" };
const juce::Identifier State::IDs::displayGroupDelay { "DisplayGroupDelay" };
const juce::Identifier State::IDs::displayLogarithmic { "DisplayLogarithmic" };
const juce::Identifier State::IDs::displayInHz { "DisplayInhZ" };

State::State (juce::ValueTree tree_)
    : TreeWrapper(tree_)
    , poleZeroState (tree_.getOrCreateChildWithName (IDs::poleZeroState, nullptr))
    , displayInDB (tree_, IDs::displayInDB, true)
    , displayGroupDelay (tree_, IDs::displayGroupDelay, false)
    , displayLogarithmic (tree_, IDs::displayLogarithmic, true)
    , displayInHz (tree_, IDs::displayInHz, true)
{

}

void State::setState (const State& other)
{
    poleZeroState.setState (other.poleZeroState);
    displayInDB.setValue (other.displayInDB.getValue());
    displayGroupDelay.setValue (other.displayGroupDelay.getValue());
    displayLogarithmic.setValue (other.displayLogarithmic.getValue());
    displayInHz.setValue (other.displayInHz.getValue());
}

