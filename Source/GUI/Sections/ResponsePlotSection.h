#pragma once

#include <JuceHeader.h>

#include "../Components/Plot.h"
#include "../../Data/Attachments/ResponsePlotAttachment.h"
#include "../LookAndFeel.h"
#include "../DSP/MathFunctions.h"

class ResponsePlotSection : public juce::Component
{
public:

    ResponsePlotSection(AudioPluginAudioProcessor& p) : state(p.state)
    {
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

        state.setOnPropertyChanged (State::IDs::displayInDB, [this]() {
            const bool shouldDisplayInDecibels = state.displayInDB.getValue();
            if (shouldDisplayInDecibels)
            {
                magnitudePlot.setRange ({ -60.0f, 12.0f });
                magnitudePlot.setYTicks ({ -40.0f, -24.0f, -12.0f, -6.0f, 0.0f, 6.0f });
                magnitudePlot.setYLabels ({"-40", "-24", "-12", "-6", "0", "+6" });
            }
            else
            {
                magnitudePlot.setRange ({ 0.0f, 2.0f });
                magnitudePlot.setYTicks ({ 0.25f, 0.5f, 0.75f, 1.0, 1.25f, 1.5f });
                magnitudePlot.setYLabels ({ "0.25","0.5", "0.75","1.0", "1.25", "1.5" });
            }
        }, true);



        phasePlot.setRange ({ -juce::MathConstants<float>::twoPi, juce::MathConstants<float>::twoPi });
        phasePlot.setYTicks ({   -juce::MathConstants<float>::pi,
                                    -juce::MathConstants<float>::halfPi, 0,
                                     juce::MathConstants<float>::halfPi,
                                     juce::MathConstants<float>::pi });
        phasePlot.setYLabels ({ "-" + piString, "-" + halfString + piString, "0", halfString + piString, piString });

        groupDelayPlot.setRange ({ -16, 16 });
        groupDelayPlot.setYTicks ({ -15, -10, -5, 0, 5, 10, 15 });
        groupDelayPlot.setYLabels ({ "", "-10", "-5", "0", "5", "10", "" });

        plotAttachment = std::make_unique<ResponsePlotAttachment> (p.state, p.filterDesign, magnitudePlot, phasePlot, groupDelayPlot);
    }

    void resized() override
    {
        auto bounds = getLocalBounds().toFloat();
        const float height = bounds.getHeight();

        const float plotHeight = (height - LAF::Layout::defaultSpacing) * 0.5f;
        auto magPlotArea = bounds.removeFromTop (plotHeight);
        magnitudePlot.setBounds (magPlotArea.toNearestInt());

        bounds.removeFromTop (LAF::Layout::defaultSpacing);
        auto phasePlotArea = bounds;
        phasePlot.setBounds (phasePlotArea.toNearestInt());
    }

    Plot magnitudePlot { "Magnitude" };
    Plot phasePlot { "Phase" };
    Plot groupDelayPlot { "Group Delay" };
    std::unique_ptr<ResponsePlotAttachment> plotAttachment;

private:
    State state;
    const juce::String piString         { juce::String (juce::CharPointer_UTF8 ("\xCF\x80")) }; // pi
    const juce::String quarterString    { juce::String (juce::CharPointer_UTF8 ("\xC2\xBC")) }; // 1/4
    const juce::String halfString       { juce::String (juce::CharPointer_UTF8 ("\xC2\xBD")) }; // 1/2
    const juce::String threeQtrString   { juce::String (juce::CharPointer_UTF8 ("\xC2\xBE")) }; // 3/4
};