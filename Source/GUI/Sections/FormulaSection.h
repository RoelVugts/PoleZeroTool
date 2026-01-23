#pragma once

#include <JuceHeader.h>

#include "../Components/BoxedLabel.h"
#include "../FilterTextFormatter.h"
#include "../../DSP/FilterDesign.h"
#include "LookAndFeel.h"

class FormulaSection : public juce::Component, private FilterDesign::Listener
{
public:

    FormulaSection(AudioPluginAudioProcessor& p) : filterDesigner (p.filterDesign)
    {
        diffEquationLabel.setFont (juce::FontOptions(11.0f));
        diffEquationLabel.setText (FilterTextFormatter::differenceEquation (p.filterDesign));
        addAndMakeVisible (diffEquationLabel);

        transferFucntionLabel.setFont (juce::FontOptions(11.0f));
        transferFucntionLabel.setText (FilterTextFormatter::transferFunction (p.filterDesign));
        addAndMakeVisible (transferFucntionLabel);

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
        transferFucntionLabel.setBounds (diffEquationArea.toNearestInt());
        bounds.removeFromLeft (LAF::Layout::defaultSpacing);
        diffEquationLabel.setBounds (bounds.toNearestInt());
    }

private:

    void filterCoefficientsChanged(FilterDesign* emitter) override
    {
        diffEquationLabel.setText (FilterTextFormatter::differenceEquation (*emitter));
        transferFucntionLabel.setText (FilterTextFormatter::transferFunction (*emitter));
    }

    void filterGainChanged(FilterDesign* emitter) override
    {
        diffEquationLabel.setText (FilterTextFormatter::differenceEquation (*emitter));
        transferFucntionLabel.setText (FilterTextFormatter::transferFunction (*emitter));
    }

    FilterDesign& filterDesigner;
    BoxedLabel transferFucntionLabel;
    BoxedLabel diffEquationLabel;

};