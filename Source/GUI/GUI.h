#pragma once

#include <JuceHeader.h>

#include "../Plugin/PluginProcessor.h"

#include "Components/VolumeMeter.h"
#include "LookAndFeel.h"
#include "Sections/FormulaSection.h"
#include "Sections/PoleZeroSection.h"
#include "Sections/ResponsePlotSection.h"
#include "Sections/SettingsSection.h"
#include "Sections/TooltipSection.h"

class GUI : public juce::Component, private juce::Timer
{
public:

    GUI(AudioPluginAudioProcessor& p)
        : processor(p)
        , poZeSection (p)
        , plotSection (p)
        , formulaSection (p)
        , settingsSection (p)
    {
        addAndMakeVisible (poZeSection);
        addAndMakeVisible (plotSection);
        addAndMakeVisible (formulaSection);
        addAndMakeVisible (settingsSection);
        addAndMakeVisible (tooltipSection);

        addAndMakeVisible (inputMeter);
        addAndMakeVisible (outputMeter);
        addAndMakeVisible (outputMeterImag);

        inputMeter.setTooltip ("Input volume meter");
        outputMeter.setTooltip ("Real output volume meter");
        outputMeterImag.setTooltip ("Imaginary output volume meter");

        startTimerHz (30);
    }

    void paint(Graphics& g) override
    {
        g.fillAll(LAF::Colours::primaryColour);
    }

    void resized() override
    {
        auto bounds = getLocalBounds().toFloat();
        const float height = bounds.getHeight();
        const float width = bounds.getWidth();

        // Border area
        auto toolTipArea = bounds.removeFromBottom (height * 0.05f);
        tooltipSection.setBounds (toolTipArea.toNearestInt());

        // Settings bar
        auto settingsArea = bounds.removeFromBottom (height * 0.05f);
        settingsSection.setBounds (settingsArea.toNearestInt());

        // Margins
        bounds.reduce (LAF::Layout::defaultSpacing, LAF::Layout::defaultSpacing);

        // Input volume meter
        const float meterWidth = width * 0.03f;
        auto inputMeterArea = bounds.removeFromLeft (meterWidth);
        inputMeter.setBounds (inputMeterArea.toNearestInt());
        bounds.removeFromLeft (LAF::Layout::defaultSpacing);

        // (Real) output meter
        auto outputMeterArea = bounds.removeFromRight (meterWidth);
        outputMeter.setBounds (outputMeterArea.toNearestInt());
        bounds.removeFromRight (LAF::Layout::defaultSpacing);

        // (Imaginary) output meter
        auto imagMeterArea = bounds.removeFromRight (meterWidth);
        outputMeterImag.setBounds (imagMeterArea.toNearestInt());
        bounds.removeFromRight (LAF::Layout::defaultSpacing);

        // Content area
        const float availableWidth = bounds.getWidth() - LAF::Layout::defaultSpacing; // Spacing between poze section and response plots
        const float availableHeight = bounds.getHeight() - LAF::Layout::defaultSpacing; // Spacing between formula and plots / table

        // Formula section
        auto formulaSectionArea =  bounds.removeFromBottom (availableHeight * 0.1f);
        formulaSection.setBounds (formulaSectionArea.toNearestInt());

        // Spacing
        bounds.removeFromBottom (LAF::Layout::defaultSpacing);

        // Pole Zero Plot + table
        auto poZeSectionArea = bounds.removeFromLeft (availableWidth * LAF::Layout::poZeContentWidthRatio);
        poZeSection.setBounds (poZeSectionArea.toNearestInt());

        // Response plots
        bounds.removeFromLeft (LAF::Layout::defaultSpacing);
        plotSection.setBounds (bounds.toNearestInt());
    }

private:

    void timerCallback() override
    {
        for (int ch = 0; ch < 2; ch++)
        {
            inputMeter.getChannelMeter (ch).setLevel (processor.inputLevel[ch].load(std::memory_order_relaxed));
            outputMeter.getChannelMeter (ch).setLevel (processor.outputLevel[ch].load(std::memory_order_relaxed));
            outputMeterImag.getChannelMeter (ch).setLevel (processor.outputLevelImag[ch].load(std::memory_order_relaxed));
        }
    }

    AudioPluginAudioProcessor& processor;
    PoleZeroSection poZeSection;
    ResponsePlotSection plotSection;
    FormulaSection formulaSection;
    SettingsSection settingsSection;
    ToolTipSection tooltipSection;

    StereoMeter inputMeter;
    StereoMeter outputMeter;
    StereoMeter outputMeterImag;
};