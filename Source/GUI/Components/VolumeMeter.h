#pragma once

#include <JuceHeader.h>

class VolumeMeter : public juce::Component, private juce::Timer
{
public:

    void paint(juce::Graphics& g) override
    {
        const float db =std::clamp(juce::Decibels::gainToDecibels (currentLevel), minLevel, maxLevel);
        const float norm = range.convertTo0to1 (db);
        const float y = (1.0f - norm) * area.getHeight() + area.getY();

        fillArea.setY(y);
        fillArea.setHeight(area.getBottom() - y);

        g.setGradientFill(gradient);
        g.fillRect(fillArea);

        startTimerHz (60);
    }

    void resized() override
    {
        area = getLocalBounds().toFloat();
        fillArea = area;

        gradient = juce::ColourGradient{
            juce::Colours::red.withAlpha(0.5f),
            area.getTopLeft(),
            juce::Colours::green.withAlpha(0.5f),
            area.getBottomLeft(),
            false
        };
        gradient.addColour(0.25f, juce::Colours::red.withAlpha(0.5f));
        gradient.addColour(0.5, juce::Colours::yellow.withAlpha(0.5f));
        gradient.addColour(0.75f, juce::Colours::green.withAlpha(0.5f));

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
    float currentLevel { -42.0f };

    float minLevel { -42.0f };
    float maxLevel { 18.0f };

    juce::NormalisableRange<float> range;

};