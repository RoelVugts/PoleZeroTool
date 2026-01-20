#pragma once

#include "GridAxis.h"

#include <JuceHeader.h>

#include "../../DSP/FilterDesign.h"

class ResponsePlot : public juce::Component, private FilterDesign::Listener
{
public:

    struct AxisLabel : public juce::Label
    {
        float value { 0.0f };
    };

    enum ColourIds
    {
        backgroundColourId      = 0x210100,
        gridColourId            = 0x210200,
        pathColourId            = 0x210300
    };

    ResponsePlot()
    {
        setColour (backgroundColourId, juce::Colours::black);
        setColour (gridColourId, juce::Colours::grey);
        setColour (pathColourId, juce::Colours::white);
    }

    void paint(juce::Graphics& g) override
    {
        g.fillAll(findColour (backgroundColourId));

        g.setColour (findColour (pathColourId));
        g.strokePath (path, pathStroke);
    }

    void resized() override
    {
        cachedValues.resize(getWidth());
        updatePath();
    }

    void setRange(const juce::NormalisableRange<float>& range)
    {
        yRange = range;
    }

    void updatePath()
    {
        path.clear();

        if (getDataFn == nullptr)
            return;

        const auto bounds = getLocalBounds();
        const int width = bounds.getWidth();

        for (int x = 0; x < width; x++)
        {
            const float angle = juce::MathConstants<float>::pi * ((float)x / (float)width);
            const float val = getDataFn(angle);
            cachedValues[x] = val;
        }

        float max = std::max(1.0f, *std::ranges::max_element(cachedValues));
        float min = std::min(0.0f, *std::ranges::min_element(cachedValues));
        yRange = { min, max };

        float val = cachedValues[0];
        float y = 1.0f - yRange.convertTo0to1 (val);
        path.startNewSubPath (0, y * (float)bounds.getHeight());

        for (int x = 1; x < width; x++)
        {
            val = cachedValues[x];
            y = 1.0f - yRange.convertTo0to1 (val);

            path.lineTo ((float)x, y * (float)bounds.getHeight());
        }

        repaint();
    }

    std::function<float(float)> getDataFn { nullptr };

private:

    juce::NormalisableRange<float> yRange;

    juce::Path path;
    juce::PathStrokeType pathStroke { 1.0f };

    std::vector<float> cachedValues;
};

//==============================================================
// Magnitude plot

class LabeledPlot : public juce::Component
{
public:

    LabeledPlot()
    {
        addAndMakeVisible (plot);
        addAndMakeVisible (grid);
    }

    void resized() override
    {
        plot.setBounds (getLocalBounds());
        grid.setBounds (getLocalBounds());
    }

private:
    ResponsePlot plot;
    GridAxis grid;
};