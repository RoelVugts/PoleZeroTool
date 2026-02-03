#pragma once

#include <JuceHeader.h>

class LogoButton : public juce::DrawableButton
{
public:

    LogoButton() : juce::DrawableButton("Logo Button", juce::DrawableButton::ButtonStyle::ImageFitted)
    {
        auto svg = juce::Drawable::createFromImageData (BinaryData::RV_Logo_svg, BinaryData::RV_Logo_svgSize);
        setImages (svg.get());
        setAlpha (0.8f);
    }

private:

    void mouseEnter(const MouseEvent& event) override
    {
        juce::DrawableButton::mouseEnter(event);
        setAlpha (1.0f);
    }

    void mouseExit(const MouseEvent& event) override
    {
        juce::DrawableButton::mouseExit(event);
        setAlpha (0.8f);
    }


};