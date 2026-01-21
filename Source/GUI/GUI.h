#pragma once

#include <JuceHeader.h>

#include "../DSP/FilterDesign.h"
#include "../DSP/MathFunctions.h"
#include "../Data/Attachments/PoZePlotAttachment.h"
#include "../Data/Attachments/ResponsePlotAttachment.h"
#include "Components/BoxedLabel.h"
#include "Components/Plot.h"
#include "Components/PoZePlot.h"
#include "Components/PoZeTable.h"
#include "LookAndFeel.h"

#include "FilterTextFormatter.h"

class GUI : public juce::Component, private FilterDesign::Listener
{
public:

    GUI(AudioPluginAudioProcessor& p)
        : state(p.state)
        , poZeTable (p.state.poleZeroState)
    {
        poZePlot.setColour (PoZePlot::ColourIds::backgroundColourId, LAF::Colours::darkBackgroundColour);
        poZePlotAttachment = std::make_unique<PoZePlotAttachment>(state.poleZeroState, poZePlot, &p.undoManager);
        addAndMakeVisible (poZePlot);

        addAndMakeVisible (poZeTable);

        for (auto* plot : juce::Array<Plot*>{&magnitudePlot, &phasePlot, &groupDelayPlot})
        {
            plot->setDomain ({ 0.0f, juce::MathConstants<float>::pi });
            plot->setXTicks ({
                MathFunctions::roundToDecimals(juce::MathConstants<float>::halfPi   * 0.5f,     2),
                MathFunctions::roundToDecimals(juce::MathConstants<float>::halfPi,              2),
                MathFunctions::roundToDecimals(juce::MathConstants<float>::pi       * 0.75f,    2),
                MathFunctions::roundToDecimals(juce::MathConstants<float>::pi,                  2) }
            );

            plot->setXLabels ({ quarterString + piString, halfString + piString, threeQtrString + piString, piString});
            addAndMakeVisible (plot);
        }

        magnitudePlot.setRange ({ -60.0f, 12.0f });
        magnitudePlot.setYTicks ({ -40.0f, -24.0f, -12.0f, -6.0f, 0.0f, 6.0f });
        magnitudePlot.setYLabels ({"-40", "-24", "-12", "-6", "0", "+6" });

        phasePlot.setRange ({ -juce::MathConstants<float>::twoPi, juce::MathConstants<float>::twoPi });
        phasePlot.setYTicks ({   -juce::MathConstants<float>::pi,
                                    -juce::MathConstants<float>::halfPi, 0,
                                     juce::MathConstants<float>::halfPi,
                                     juce::MathConstants<float>::pi });
        phasePlot.setYLabels ({ "-" + piString, "-" + halfString + piString, "0", halfString + piString, piString });

        groupDelayPlot.setRange ({ -16, 16 });
        groupDelayPlot.setYTicks ({ -15, -10, -5, 0, 5, 10, 15 });
        groupDelayPlot.setYLabels ({ "", "-10", "-5", "0", "5", "10", "" });

        plotAttachment = std::make_unique<ResponsePlotAttachment> (p.filterDesign, magnitudePlot, phasePlot, groupDelayPlot);

        diffEquationLabel.setFont (juce::FontOptions(11.0f));
        diffEquationLabel.setText (FilterTextFormatter::differenceEquation (p.filterDesign));
        addAndMakeVisible (diffEquationLabel);

        transerFunctionText.setFont (juce::FontOptions(11.0f));
        transerFunctionText.setText (FilterTextFormatter::differenceEquation (p.filterDesign));
        addAndMakeVisible (transerFunctionText);

        p.filterDesign.addListener (this);
    }

    ~GUI() override
    {
        //TODO: Remove listener
    }

    void resized() override
    {
        auto bounds = getLocalBounds().toFloat();

        constexpr float spacing = 6.0f;

        // ---- Side meters ----
        bounds.removeFromLeft (bounds.getWidth() * 0.05f + spacing);
        bounds.removeFromRight (bounds.getWidth() * 0.08f + spacing);

        // ---- Header ----
        bounds.removeFromTop (bounds.getHeight() * 0.06f + spacing);

        // ---- Footer / Formula ----
        auto formulaArea = bounds.removeFromBottom (bounds.getHeight() * 0.1f);
        bounds.removeFromBottom (spacing);

        // ---- Main content ----
        auto content = bounds;

        // Columns
        const float leftColumnWidth = content.getWidth() * 0.35f;
        auto leftColumn  = content.removeFromLeft (leftColumnWidth);
        content.removeFromLeft (spacing);
        auto rightColumn = content;

        // ---- Right column (plots) ----
        const float plotHeight =
            (rightColumn.getHeight() - spacing) * 0.5f;

        auto magnitudePlotArea = rightColumn.removeFromTop (plotHeight);
        magnitudePlot.setBounds (magnitudePlotArea.toNearestInt());

        rightColumn.removeFromTop (spacing);

        auto phasePlotArea = rightColumn;
        phasePlot.setBounds (phasePlotArea.toNearestInt());
        groupDelayPlot.setBounds (phasePlotArea.toNearestInt());

        // ---- Left column ----
        // XY pad must be square
        const float xyPadSize = magnitudePlot.getHeight();

        auto xyPadArea = leftColumn.removeFromTop (xyPadSize);
        xyPadArea.setWidth (xyPadSize);
        poZePlot.setBounds (xyPadArea.toNearestInt());

        leftColumn.removeFromTop (spacing);

        // Table aligned with phase plot
        auto tableArea = leftColumn;
        tableArea.setWidth (xyPadSize);
        tableArea.setY (phasePlotArea.getY());
        tableArea.setHeight (phasePlotArea.getHeight());
        poZeTable.setBounds (tableArea.toNearestInt());

        auto diffEquationArea = formulaArea.removeFromRight ((float)phasePlot.getWidth());
        diffEquationLabel.setBounds (diffEquationArea.toNearestInt());

        auto transferFunctionArea = formulaArea.removeFromLeft ((float)poZeTable.getWidth());
        transerFunctionText.setBounds (transferFunctionArea.toNearestInt());
    }

private:

    void filterCoefficientsChanged(FilterDesign* emitter) override
    {
        diffEquationLabel.setText (FilterTextFormatter::differenceEquation (*emitter));
        transerFunctionText.setText (FilterTextFormatter::transferFunction (*emitter));
    }

    State state;
    PoZePlot poZePlot;
    std::unique_ptr<PoZePlotAttachment> poZePlotAttachment;

    PoZeTable poZeTable;
    Plot magnitudePlot { "Magnitude" };
    Plot phasePlot { "Phase" };
    Plot groupDelayPlot { "Group Delay" };
    std::unique_ptr<ResponsePlotAttachment> plotAttachment;

    BoxedLabel transerFunctionText;
    BoxedLabel diffEquationLabel;

    const juce::String piString         { juce::String (juce::CharPointer_UTF8 ("\xCF\x80")) };
    const juce::String quarterString    { juce::String (juce::CharPointer_UTF8 ("\xC2\xBC")) };
    const juce::String halfString       { juce::String (juce::CharPointer_UTF8 ("\xC2\xBD")) }; // ½
    const juce::String threeQtrString   { juce::String (juce::CharPointer_UTF8 ("\xC2\xBE")) }; // ¾
};