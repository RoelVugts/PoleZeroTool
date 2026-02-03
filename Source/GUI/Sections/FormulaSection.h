#pragma once

#include <JuceHeader.h>

#include "../../DSP/FilterDesign.h"
#include "../Components/BoxedLabel.h"
#include "../FilterTextFormatter.h"
#include "../LookAndFeel.h"

class FormulaSection : public juce::Component, private FilterDesign::Listener
{
public:

    FormulaSection(AudioPluginAudioProcessor& p) : filterDesigner (p.filterDesign)
    {
        diffEquationLabel.setFont (juce::FontOptions(11.0f));
        diffEquationLabel.setText (FilterTextFormatter::differenceEquation (p.filterDesign));
        diffEquationLabel.setTooltip ("The difference equation");
        diffEquationLabel.setColour (BoxedLabel::ColourIds::backgroundColourId, LAF::Colours::secondaryColour);
        diffEquationLabel.setColour (BoxedLabel::ColourIds::outlineColourId, LAF::Colours::buttonOutlineColour);
        addAndMakeVisible (diffEquationLabel);

        transferFunctionLabel.setFont (juce::FontOptions(11.0f));
        transferFunctionLabel.setText (FilterTextFormatter::transferFunction (p.filterDesign));
        transferFunctionLabel.setTooltip ("The transfer function");
        transferFunctionLabel.setColour (BoxedLabel::ColourIds::backgroundColourId, LAF::Colours::secondaryColour);
        transferFunctionLabel.setColour (BoxedLabel::ColourIds::outlineColourId, LAF::Colours::buttonOutlineColour);
        addAndMakeVisible (transferFunctionLabel);

        filterDesigner.addListener (this);
    }

    ~FormulaSection() override
    {
        filterDesigner.removeListener (this);
    }

    void resized() override
    {
        auto bounds = getLocalBounds().toFloat();
        const float width = bounds.getWidth();

        const float transferFunctionWidth = (width - LAF::Layout::defaultSpacing) * LAF::Layout::poZeContentWidthRatio;
        auto diffEquationArea = bounds.removeFromLeft (transferFunctionWidth);
        transferFunctionLabel.setBounds (diffEquationArea.toNearestInt());
        bounds.removeFromLeft (LAF::Layout::defaultSpacing);
        diffEquationLabel.setBounds (bounds.toNearestInt());
    }

private:

    void filterCoefficientsChanged(FilterDesign*) override
    {
        juce::MessageManager::callAsync ([this]() {
            diffEquationLabel.setText (FilterTextFormatter::differenceEquation (filterDesigner));
            transferFunctionLabel.setText (FilterTextFormatter::transferFunction (filterDesigner));
        });
    }

    void filterGainChanged(FilterDesign*) override
    {
        juce::MessageManager::callAsync ([this]() {
            diffEquationLabel.setText (FilterTextFormatter::differenceEquation (filterDesigner));
            transferFunctionLabel.setText (FilterTextFormatter::transferFunction (filterDesigner));
        });
    }

    FilterDesign& filterDesigner;
    BoxedLabel transferFunctionLabel;
    BoxedLabel diffEquationLabel;

};