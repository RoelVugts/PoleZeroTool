#include "PoleZeroState.h"

#include "../GUI/Components/PoZePlot.h"

//==============================================================
const juce::Identifier PointState::IDs::type { "Point" };
const juce::Identifier PointState::IDs::isPole { "IsPole" };
const juce::Identifier PointState::IDs::value { "Value" };

PointState::PointState (juce::ValueTree tree_)
    : TreeWrapper (tree_)
    , isPole (tree_, IDs::isPole, PoZePlot::Point::Type::zero)
    , value (tree_, IDs::value, { 0.0f, 0.0f })
{

}

void PointState::setState (const PointState& other)
{
    isPole.setValue (other.isPole.getValue());
    value.setValue (other.value.getValue());
}

//==============================================================
const juce::Identifier PoleZeroState::IDs::type { "PoleZeroState" };
const juce::Identifier PoleZeroState::IDs::points { "Points" };

PoleZeroState::PoleZeroState (juce::ValueTree tree_)
    : TreeWrapper(tree_)
    , points(tree_.getOrCreateChildWithName (IDs::points, nullptr), PointState::IDs::type)
{

}

void PoleZeroState::setState (const PoleZeroState& other)
{
    points.setState (other.points);
}
