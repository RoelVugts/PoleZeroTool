#pragma once

#include <JuceHeader.h>

#include "../../Data/Attachments/DragBoxAttachment.h"
#include "../Components/DragBox.h"
#include "../LookAndFeel.h"

class SettingsSection : public juce::Component
{
public:

    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;

    SettingsSection(AudioPluginAudioProcessor& p)
    {
        gainBox.setNumDecimalsToDisplay (5);
        gainBox.setColour (DragBox::ColourIds::backgroundColourId, juce::Colour(30, 30, 30));
        gainAttachment = DragBoxAttachment::makeAttachment (p.apvts, paramID[PoZeParamID::gain], gainBox);
        addAndMakeVisible (gainBox);

        autoNormalizeBtn.setColour (juce::TextButton::ColourIds::buttonColourId, LAF::Colours::buttonOffColour);
        autoNormalizeBtn.setColour (juce::TextButton::ColourIds::buttonOnColourId, LAF::Colours::buttonOnColour);
        autoNormalizeBtn.setColour (juce::TextButton::ColourIds::textColourOffId, LAF::Colours::textColour);
        autoNormalizeBtn.setColour (juce::TextButton::ColourIds::textColourOnId, juce::Colours::white);
        autoNormalizeBtn.setClickingTogglesState (true);
        autoNormalizeAttachment = std::make_unique<ButtonAttachment>(p.apvts, paramID[PoZeParamID::autoNormalise], autoNormalizeBtn);
        addAndMakeVisible (autoNormalizeBtn);
    }


    void paint(Graphics& g) override
    {
        g.setColour (juce::Colours::black);
        g.fillRect (getLocalBounds());
    }

    void resized() override
    {
        auto bounds = getLocalBounds().toFloat();
        bounds.reduce(LAF::Layout::defaultSpacing * 2, bounds.getHeight() * 0.2f);

        static constexpr int numButtons = 7;
        constexpr float spacingWidth = (float)(numButtons - 1) * LAF::Layout::defaultSpacing;
        const float btnSize = (bounds.getWidth() - spacingWidth) / numButtons;

        auto gainBoxArea = bounds.removeFromLeft (btnSize);
        gainBox.setBounds (gainBoxArea.toNearestInt());

        bounds.removeFromLeft (LAF::Layout::defaultSpacing);
        auto normalizeBtnArea = bounds.removeFromLeft (btnSize);
        autoNormalizeBtn.setBounds (normalizeBtnArea.toNearestInt());
    }

private:
    DragBox gainBox;
    juce::TextButton autoNormalizeBtn { "Normalize" };
    std::unique_ptr<DragBoxAttachment> gainAttachment;
    std::unique_ptr<ButtonAttachment> autoNormalizeAttachment;
};