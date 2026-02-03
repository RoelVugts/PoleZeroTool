#pragma once

#include "../LookAndFeel.h"

#include <JuceHeader.h>

class VersionDialog : public juce::Component
{
public:

    VersionDialog()
    {
        projectBtn.setColour (juce::HyperlinkButton::ColourIds::textColourId, LAF::Colours::textColour);
        addAndMakeVisible (projectBtn);
        addAndMakeVisible (companyBtn);
        addAndMakeVisible (nameBtn);
    }

    void paint(Graphics& g) override
    {
        static constexpr float lineThickness = 2.0f;

        auto bounds = getLocalBounds().toFloat();
        bounds = bounds.reduced (lineThickness * 0.5f);

        const float cornerSize = std::min(bounds.getHeight(), bounds.getWidth()) * 0.1f;

        g.setColour (LAF::Colours::darkBackgroundColour);
        g.fillRoundedRectangle (bounds, cornerSize);

        g.setColour (LAF::Colours::buttonOutlineColour);
        g.drawRoundedRectangle (bounds, cornerSize, lineThickness);

        bounds = bounds.reduced (LAF::Layout::defaultSpacing);

        const float contentHeight = bounds.getHeight() - LAF::Layout::defaultSpacing * 2.0f;
        auto titleTextArea = bounds.removeFromTop (contentHeight * 0.4f);
        projectBtn.setBounds (titleTextArea.toNearestInt());
        bounds.removeFromTop (LAF::Layout::defaultSpacing);

        auto companyTextArea = bounds.removeFromTop (contentHeight * 0.2f);
        companyBtn.setBounds (companyTextArea.toNearestInt());

        auto nameTextArea = bounds.removeFromTop (contentHeight * 0.2f);
        nameBtn.setBounds (nameTextArea.toNearestInt());
        bounds.removeFromTop (LAF::Layout::defaultSpacing);

        auto versionArea = bounds.removeFromBottom(contentHeight * 0.2f);
        g.setColour (LAF::Colours::textColour.darker (0.8f));
        g.setFont (versionArea.getHeight() * 0.7f);
        g.drawFittedText (ProjectInfo::versionString, versionArea.toNearestInt(), juce::Justification::centred, 1, 0.9f);
    }

private:
    juce::HyperlinkButton projectBtn { "PoZeTool", juce::URL("https://github.com/RoelVugts/PoleZeroTool") };
    juce::HyperlinkButton companyBtn { "RV Audio Design", juce::URL("https://rvaudiodesign.com") };
    juce::HyperlinkButton nameBtn { "Roel Vugts", juce::URL("https://roelvugts.nl") };
};