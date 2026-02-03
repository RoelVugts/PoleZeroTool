#pragma once

#include <JuceHeader.h>

#include "../Components/VersionDialog.h"

class Overlay : public juce::Component
{
public:

    Overlay()
    {
        addAndMakeVisible (dialog);
    }

    void paint(Graphics& g) override
    {
        g.fillAll(juce::Colours::black.withAlpha (0.6f));
    }

    void resized() override
    {
        auto bounds = getLocalBounds().toFloat();
        bounds = bounds.withSizeKeepingCentre (bounds.getWidth() * 0.3f, bounds.getHeight() * 0.25f);
        dialog.setBounds (bounds.toNearestInt());
    }

private:

    void mouseDown(const MouseEvent&) override
    {
        setVisible (false);
    }

    VersionDialog dialog;
};