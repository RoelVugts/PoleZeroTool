#include "State.h"

const juce::Identifier State::IDs::type { "State" };

State::State (juce::ValueTree tree_)
    : TreeWrapper(tree_)
    , poleZeroState (tree_)
{

}

void State::setState (const State& other)
{
    poleZeroState.setState (other.poleZeroState);
}

