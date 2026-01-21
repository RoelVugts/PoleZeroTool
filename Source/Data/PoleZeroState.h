#pragma once

#include "../GUI/Components/PoZePlot.h"

#include "Utils/TreeListWrapper.h"
#include "Utils/TreePropertyWrapper.h"
#include "Utils/TreeWrapper.h"
#include "Utils/VariantConverters.h"
#include <JuceHeader.h>

#include <complex>

class PointState : public TreeWrapper
{
public:

    struct IDs
    {
        static const juce::Identifier type;
        static const juce::Identifier pointType;
        static const juce::Identifier real;
        static const juce::Identifier imag;
        static const juce::Identifier conjugateIndex;
    };

    PointState(juce::ValueTree tree);

    void setState(const PointState& other);

    TreePropertyWrapper<PoZePlot::Point::Type> pointType;
    TreePropertyWrapper<double> real;
    TreePropertyWrapper<double> imag;
    TreePropertyWrapper<int> conjugateIndex;
};

class PoleZeroState : public TreeWrapper
{
public:

    struct IDs
    {
        static const juce::Identifier type;
        static const juce::Identifier points;
    };

    PoleZeroState(juce::ValueTree tree);

    void setState(const PoleZeroState& other);

    TreeListWrapper<PointState> points;
};