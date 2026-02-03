#pragma once

#include <JuceHeader.h>

class VolumeMeter : public juce::Component, private juce::Timer, public juce::SettableTooltipClient
{
public:

    enum ColourIds
    {
        backgroundColourId      = 0x201501
    };

    VolumeMeter()
    {
        setColour (backgroundColourId, juce::Colours::black);
    }

    void paint(juce::Graphics& g) override
    {
        const float db = std::clamp(juce::Decibels::gainToDecibels (currentLevel, (float)MINUS_INFINITY_DB), minLevel, maxLevel);
        const float norm = range.convertTo0to1 (db);
        const float y = (1.0f - norm) * area.getHeight() + area.getY();

        fillArea.setY(y);
        fillArea.setHeight(area.getBottom() - y);

        g.fillAll(findColour (backgroundColourId));

        g.setGradientFill(gradient);
        g.fillRect(fillArea);

        startTimerHz (60);
    }

    void resized() override
    {
        area = getLocalBounds().toFloat();
        fillArea = area;

        gradient = juce::ColourGradient{
            red.withAlpha(0.5f),
            area.getTopLeft(),
            green.withAlpha(0.5f),
            area.getBottomLeft(),
            false
        };
        gradient.addColour(0.25f, red);
        gradient.addColour(0.5, orange);
        gradient.addColour(0.75f, green);

        range = juce::NormalisableRange<float> { minLevel, maxLevel };
    }

    void setLevel(float amp)
    {
        if (amp > currentLevel)
            currentLevel = amp;
    }

private:

    void timerCallback() override
    {
        currentLevel *= 0.93f;
        repaint();
    }

    juce::Rectangle<float> area, fillArea;
    juce::ColourGradient gradient;
    float currentLevel { -48.0f };

    float minLevel { -42.0f };
    float maxLevel { 12.0f };

    juce::NormalisableRange<float> range;

    const juce::Colour red    { 201, 74, 74  };
    const juce::Colour orange { 242, 165, 65 };
    const juce::Colour green  { 76, 175, 80 };
};


//===================================================================
// Stereo meter
class StereoMeter : public juce::Component, public juce::SettableTooltipClient
{
public:

    StereoMeter()
    {
        for (auto& meter : meters)
            addAndMakeVisible (meter);
    }

    void setTooltip(const String &newTooltip) override
    {
        for (auto& meter : meters)
            meter.setTooltip(newTooltip);
    }

    void paint(Graphics& g) override
    {
        g.fillAll(meters[0].findColour (VolumeMeter::backgroundColourId));
    }

    void resized() override
    {
        auto bounds = getLocalBounds().toFloat();
        auto leftMeterArea = bounds.removeFromLeft (bounds.getWidth() * 0.5f);
        meters[0].setBounds (leftMeterArea.toNearestInt());
        meters[1].setBounds (bounds.toNearestInt());
    }

    VolumeMeter& getChannelMeter(int index)
    {
        jassert(isPositiveAndBelow (index, numChannels));

        return meters[(size_t)index];
    }

private:
    static constexpr int numChannels { 2 };
    std::array<VolumeMeter, numChannels> meters;
};