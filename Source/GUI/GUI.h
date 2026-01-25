#pragma once

#include <JuceHeader.h>

#include "../Plugin/PluginProcessor.h"

#include "Components/VolumeMeter.h"
#include "LookAndFeel.h"
#include "Sections/FormulaSection.h"
#include "Sections/PoleZeroSection.h"
#include "Sections/ResponsePlotSection.h"
#include "Sections/SettingsSection.h"

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
        addAndMakeVisible (inputMeter[0]);
        addAndMakeVisible (inputMeter[1]);
        addAndMakeVisible (outputMeter[0]);
        addAndMakeVisible (outputMeter[1]);

        startTimerHz (30);
    }

    void resized() override
    {
        auto bounds = getLocalBounds().toFloat();
        const float height = bounds.getHeight();
        const float width = bounds.getWidth();

        // Border area
        auto headerArea = bounds.removeFromTop (height * 0.05f);
        auto settingsArea = bounds.removeFromBottom (height * 0.05f);
        settingsSection.setBounds (settingsArea.toNearestInt());

        auto inputMeterArea = bounds.removeFromLeft (width * 0.05f);
        inputMeterArea = inputMeterArea.reduced(LAF::Layout::defaultSpacing);
        auto leftMeterArea = inputMeterArea.removeFromLeft (inputMeterArea.getWidth() * 0.5f);
        inputMeter[0].setBounds (leftMeterArea.toNearestInt());
        inputMeter[1].setBounds (inputMeterArea.toNearestInt());

        auto outputMeterArea = bounds.removeFromRight (width * 0.1f);
        outputMeterArea = outputMeterArea.reduced(LAF::Layout::defaultSpacing);
        leftMeterArea = outputMeterArea.removeFromLeft (outputMeterArea.getWidth() * 0.5f);
        outputMeter[0].setBounds (leftMeterArea.toNearestInt());
        outputMeter[1].setBounds (outputMeterArea.toNearestInt());

        // Margins
        bounds.removeFromBottom (LAF::Layout::defaultSpacing);
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
        }

    }

    AudioPluginAudioProcessor& processor;
    PoleZeroSection poZeSection;
    ResponsePlotSection plotSection;
    FormulaSection formulaSection;
    SettingsSection settingsSection;
    VolumeMeter inputMeter[2];
    VolumeMeter outputMeter[2];
};