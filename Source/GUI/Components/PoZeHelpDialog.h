#pragma once

#include <JuceHeader.h>

class PoZeHelpDialog : public juce::Component
{
public:

    PoZeHelpDialog()
    {
        controlText.setFont (juce::FontOptions(12.0f));
        controlText.setText ("Controls:", juce::dontSendNotification);
        addAndMakeVisible (controlText);

        #if JUCE_MAC
            const juce::String ctrlString = "Command";
        #else
            const juce::String ctrlString = "Ctrl";
        #endif

        juce::String cmdText;
        cmdText << "- " + ctrlString + "-Click:\n";
        cmdText << "- Shift-Click:\n";
        cmdText << "- Alt-Click:\n";
        cmdText << "- " + ctrlString + "-Shift-Click:\n";
        cmdText << "- " + ctrlString + "-drag:\n";
        cmdText << "- R+Drag:\n";
        cmdText << "- M+Drag:";

        commandText.setFont (juce::FontOptions(12.0f));
        commandText.setText (cmdText, juce::dontSendNotification);
        addAndMakeVisible (commandText);

        juce::String descriptionText;
        descriptionText << "Add Pole\n";
        descriptionText << "Add Zero\n";
        descriptionText << "Remove Pole / Zero\n";
        descriptionText << "Create conjugate\n";
        descriptionText << "Constrain to real axis\n";
        descriptionText << "Rotate angle\n";
        descriptionText << "Scale magnitude";

        descText.setFont (juce::FontOptions(12.0f));
        descText.setText (descriptionText, juce::dontSendNotification);
        addAndMakeVisible (descText);
    }

    void resized() override
    {
        auto bounds = getLocalBounds().toFloat();
        auto controlArea = bounds.removeFromTop (controlText.getFont().getHeight() * 2);
        controlText.setBounds (controlArea.toNearestInt());

        const float availableWidth = bounds.getWidth() - LAF::Layout::defaultSpacing;
        commandText.setBounds (bounds.removeFromLeft(availableWidth * 0.5f).toNearestInt());
        descText.setBounds (bounds.removeFromRight (availableWidth * 0.5f).toNearestInt());
    }

    int getPreferredWidth()
    {
        int maxWidth = 0;

        const juce::StringArray& cmdLines = juce::StringArray::fromLines (commandText.getText ());
        const juce::StringArray& descLines = juce::StringArray::fromLines (descText.getText ());

        for (int i = 0; i < (int)cmdLines.size(); i++)
        {
            const int lineWidth = 30 + juce::GlyphArrangement::getStringWidthInt (getLookAndFeel().getLabelFont (commandText), cmdLines[i])
                                     + juce::GlyphArrangement::getStringWidthInt (getLookAndFeel().getLabelFont (descText), descLines[i]);

            maxWidth = std::max(lineWidth, maxWidth);
        }

        return maxWidth + (int)LAF::Layout::defaultSpacing * 4;
    }

    int getPreferredHeight() const
    {
        const float lineHeight = commandText.getFont().getHeight();
        float totalHeight = LAF::Layout::defaultSpacing * 4.0f;;
        totalHeight += lineHeight * (float)juce::StringArray::fromLines (commandText.getText ()).size();

        return (int)std::ceil(totalHeight);
    }

private:
    juce::Label controlText { "Controls:"};
    juce::Label commandText;
    juce::Label descText;
};