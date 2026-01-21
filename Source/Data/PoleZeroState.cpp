#include "PoleZeroState.h"

#include "../GUI/Components/PoZePlot.h"

//==============================================================
const juce::Identifier PointState::IDs::type { "Point" };
const juce::Identifier PointState::IDs::pointType { "Type" };
const juce::Identifier PointState::IDs::real { "Real" };
const juce::Identifier PointState::IDs::imag { "Imag" };
const juce::Identifier PointState::IDs::conjugateIndex { "ConjugateIndex" };

PointState::PointState (juce::ValueTree tree_)
    : TreeWrapper (tree_)
    , pointType (tree_, IDs::pointType, PoZePlot::Point::Type::zero)
    , real (tree_, IDs::real,  0.0)
    , imag( tree_, IDs::imag, 0.0)
    , conjugateIndex (tree_, IDs::conjugateIndex, -1)
{

}

void PointState::setState (const PointState& other)
{
    pointType.setValue (other.pointType.getValue());
    real.setValue (other.real.getValue());
    imag.setValue (other.imag.getValue());
    conjugateIndex.setValue (other.conjugateIndex.getValue());
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
