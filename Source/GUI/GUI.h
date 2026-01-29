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

        addAndMakeVisible (inputMeter[0]);
        addAndMakeVisible (inputMeter[1]);
        addAndMakeVisible (outputMeter[0]);
        addAndMakeVisible (outputMeter[1]);
        addAndMakeVisible (outputMeterImag[0]);
        addAndMakeVisible (outputMeterImag[1]);

        for (auto& ch : inputMeter)
            ch.setTooltip ("Input volume meter");

        for (auto& ch : outputMeter)
            ch.setTooltip ("Real output volume meter");

        for (auto& ch : outputMeterImag)
            ch.setTooltip ("Imaginary output volume meter");

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

        auto settingsArea = bounds.removeFromBottom (height * 0.05f);
        settingsSection.setBounds (settingsArea.toNearestInt());

        auto inputMeterArea = bounds.removeFromLeft (width * 0.05f);
        inputMeterArea = inputMeterArea.reduced(LAF::Layout::defaultSpacing);
        auto leftMeterArea = inputMeterArea.removeFromLeft (inputMeterArea.getWidth() * 0.5f);
        inputMeter[0].setBounds (leftMeterArea.toNearestInt());
        inputMeter[1].setBounds (inputMeterArea.toNearestInt());

        auto outputMeterArea = bounds.removeFromRight (width * 0.1f);
        outputMeterArea = outputMeterArea.reduced(LAF::Layout::defaultSpacing);
        const float meterWidth = (outputMeterArea.getWidth() - LAF::Layout::defaultSpacing) * 0.5f;

        auto realMeterArea = outputMeterArea.removeFromRight (meterWidth);
        leftMeterArea = realMeterArea.removeFromLeft (realMeterArea.getWidth() * 0.5f);
        outputMeter[0].setBounds (leftMeterArea.toNearestInt());
        outputMeter[1].setBounds (realMeterArea.toNearestInt());

        outputMeterArea.removeFromRight (LAF::Layout::defaultSpacing);

        auto imagMeterArea = outputMeterArea.removeFromLeft (meterWidth);
        leftMeterArea = imagMeterArea.removeFromLeft (imagMeterArea.getWidth() * 0.5f);
        outputMeterImag[0].setBounds (leftMeterArea.toNearestInt());
        outputMeterImag[1].setBounds (imagMeterArea.toNearestInt());

        // Margins
        bounds.removeFromBottom (LAF::Layout::defaultSpacing * 0.5f);
        bounds.removeFromTop (LAF::Layout::defaultSpacing);

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
            inputMeter[ch].setLevel (processor.inputLevel[ch]);
            outputMeter[ch].setLevel (processor.outputLevel[ch]);
            outputMeterImag[ch].setLevel (processor.outputLevelImag[ch]);
        }

    }

    AudioPluginAudioProcessor& processor;
    PoleZeroSection poZeSection;
    ResponsePlotSection plotSection;
    FormulaSection formulaSection;
    SettingsSection settingsSection;
    ToolTipSection tooltipSection;

    VolumeMeter inputMeter[2];
    VolumeMeter outputMeter[2];
    VolumeMeter outputMeterImag[2];
};