#pragma once

#include <JuceHeader.h>

class ResponsePlot : public juce::Component
{
public:

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

    void paint(Graphics& g) override
    {
        auto cornerSize = std::min(getWidth(), getHeight()) * 0.08f;
        g.setColour (findColour (backgroundColourId));
        g.fillRoundedRectangle(getLocalBounds().toFloat(), cornerSize);

        g.setColour (findColour (pathColourId));
    }

    void setRange(const juce::NormalisableRange<float>& xRange_, const juce::NormalisableRange<float>& yRange_)
    {
        xRange = xRange_;
        yRange = yRange_;
    }

private:
    juce::NormalisableRange<float> xRange;
    juce::NormalisableRange<float> yRange;

    juce::Path path;
    juce::PathStrokeType pathStroke { 1.0f };
};