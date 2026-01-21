#pragma once

#include <JuceHeader.h>

#include "../DSP/MathFunctions.h"
#include "../Data/Attachments/PoZePlotAttachment.h"
#include "../Data/Attachments/ResponsePlotAttachment.h"
#include "Components/Plot.h"
#include "Components/PoZePlot.h"
#include "Components/PoZeTable.h"
#include "LookAndFeel.h"

class GUI : public juce::Component
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

        for (auto* plot : juce::Array<Plot*>{&magnitudePlot, &phasePlot})
        {
            plot->setDomain ({ 0.0f, juce::MathConstants<float>::pi });
            plot->setXTicks ({
                MathFunctions::roundToDecimals(juce::MathConstants<float>::halfPi   * 0.5f,     2),
                MathFunctions::roundToDecimals(juce::MathConstants<float>::halfPi,              2),
                MathFunctions::roundToDecimals(juce::MathConstants<float>::pi       * 0.75f,    2),
                MathFunctions::roundToDecimals(juce::MathConstants<float>::pi,                  2) }
            );

            plot->setXLabels ({ quarterString + piString, halfString + piString, threeQtrString + piString, piString});
        }

        magnitudePlot.setRange ({ -60.0f, 12.0f });
        magnitudePlot.setYTicks ({ -40.0f, -24.0f, -12.0f, -6.0f, 0.0f, 6.0f });
        magnitudePlot.setYLabels ({"-40", "-24", "-12", "-6", "0", "+6" });
        addAndMakeVisible (magnitudePlot);

        phasePlot.setRange ({ -juce::MathConstants<float>::twoPi, juce::MathConstants<float>::twoPi });
        phasePlot.setYTicks ({   -juce::MathConstants<float>::pi,
                                    -juce::MathConstants<float>::halfPi, 0,
                                     juce::MathConstants<float>::halfPi,
                                     juce::MathConstants<float>::pi });
        phasePlot.setYLabels ({ "-" + piString, "-" + halfString + piString, "0", halfString + piString, piString });
        addAndMakeVisible (phasePlot);

        magAttachment = std::make_unique<ResponsePlotAttachment> (magnitudePlot, p.filterDesign, true);
        phaseAttachment = std::make_unique<ResponsePlotAttachment> (phasePlot, p.filterDesign, false);
    }

    void resized() override
    {
        auto bounds = getLocalBounds().toFloat();
        float width = bounds.getWidth();
        float height = bounds.getHeight();

        auto headerArea = bounds.removeFromTop (height * 0.05f);
        auto inputMeterArea = bounds.removeFromLeft (width * 0.1f);
        auto outputMeterArea = bounds.removeFromRight (width * 0.1f);
        bounds.removeFromBottom (height * 0.05f);

        const float xyPadSize = bounds.getHeight() * 0.48f;
        auto leftArea = bounds.removeFromLeft (xyPadSize);
        auto xyPadArea = leftArea.removeFromTop (xyPadSize);
        poZePlot.setBounds (xyPadArea.toNearestInt());

        leftArea.removeFromTop (height * 0.02f);        // Spacing


        auto tableArea = leftArea;
        poZeTable.setBounds (tableArea.toNearestInt());

        bounds.removeFromLeft (width * 0.02f);

        auto magnitudePlotArea = bounds.removeFromTop (xyPadSize);
        magnitudePlot.setBounds (magnitudePlotArea.toNearestInt());

        bounds.removeFromTop (height * 0.02f);        // Spacing

        auto phasePlotArea = bounds.removeFromTop (tableArea.getHeight());
        phasePlot.setBounds (phasePlotArea.toNearestInt());
    }

private:

    State state;
    PoZePlot poZePlot;
    std::unique_ptr<PoZePlotAttachment> poZePlotAttachment;

    PoZeTable poZeTable;
    Plot magnitudePlot { "Magnitude" };
    Plot phasePlot { "Phase" };
    std::unique_ptr<ResponsePlotAttachment> magAttachment, phaseAttachment;


    const juce::String piString         { juce::String (juce::CharPointer_UTF8 ("\xCF\x80")) };
    const juce::String quarterString    { juce::String (juce::CharPointer_UTF8 ("\xC2\xBC")) };
    const juce::String halfString       { juce::String (juce::CharPointer_UTF8 ("\xC2\xBD")) }; // ½
    const juce::String threeQtrString   { juce::String (juce::CharPointer_UTF8 ("\xC2\xBE")) }; // ¾
};