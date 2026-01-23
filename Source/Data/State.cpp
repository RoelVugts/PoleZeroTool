#include "State.h"

const juce::Identifier State::IDs::type { "State" };
const juce::Identifier State::IDs::displayInDB { "DisplayInDB" };

State::State (juce::ValueTree tree_)
    : TreeWrapper(tree_)
    , poleZeroState (tree_)
    , displayInDB (tree_, IDs::displayInDB, true)
{

}

void State::setState (const State& other)
{
    poleZeroState.setState (other.poleZeroState);
    displayInDB.setValue (other.displayInDB.getValue());
}

