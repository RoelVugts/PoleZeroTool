#pragma once

#include <JuceHeader.h>

#include "../../Data/Attachments/DragBoxAttachment.h"
#include "../../Data/State.h"
#include "../Components/DragBox.h"
#include "../LookAndFeel.h"

class SettingsSection : public juce::Component
{
public:

    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;

    SettingsSection(AudioPluginAudioProcessor& p)
        : state(p.state)
    {
        gainBox.setNumDecimalsToDisplay (5);
        gainBox.setColour (DragBox::ColourIds::backgroundColourId, juce::Colour(30, 30, 30));
        gainAttachment = DragBoxAttachment::makeAttachment (p.apvts, paramID[PoZeParamID::gain], gainBox);
        addAndMakeVisible (gainBox);

        for (auto* button : juce::Array<juce::TextButton*>{&decibelBtn, &groupDelayBtn, &logarithmicBtn, &unitBtn})
        {
            button->setColour (juce::TextButton::ColourIds::buttonColourId, LAF::Colours::buttonOffColour);
            button->setColour (juce::TextButton::ColourIds::buttonOnColourId, LAF::Colours::buttonOnColour);
            button->setColour (juce::TextButton::ColourIds::textColourOffId, juce::Colours::white);
            button->setColour (juce::TextButton::ColourIds::textColourOnId, juce::Colours::white);
            button->setClickingTogglesState (true);
            addAndMakeVisible (button);
        }

        for (auto* button : juce::Array<juce::TextButton*>{&autoNormalizeBtn, &bypassBtn})
        {
            button->setColour (juce::TextButton::ColourIds::buttonColourId, LAF::Colours::buttonOffColour);
            button->setColour (juce::TextButton::ColourIds::buttonOnColourId, LAF::Colours::buttonOnColour);
            button->setColour (juce::TextButton::ColourIds::textColourOffId, LAF::Colours::textColour);
            button->setColour (juce::TextButton::ColourIds::textColourOnId, juce::Colours::white);
            button->setClickingTogglesState (true);
            addAndMakeVisible (button);
        }

        // autoNormalizeAttachment = std::make_unique<ButtonAttachment>(p.apvts, paramID[PoZeParamID::autoNormalise], autoNormalizeBtn);

        decibelBtn.onClick = [this]() {
            const bool isOn = decibelBtn.getToggleState();
            decibelBtn.setButtonText (isOn ? "dB" : "Amp");
        };
        decibelAttachment = std::make_unique<ButtonPropertyAttachment>(state.displayInDB, decibelBtn, nullptr);

        groupDelayBtn.onClick = [this]() {
            const bool isOn = groupDelayBtn.getToggleState();
            groupDelayBtn.setButtonText (isOn ? "Phase" : "Delay");
        };
        groupDelayAttachment = std::make_unique<ButtonPropertyAttachment>(state.displayGroupDelay, groupDelayBtn, nullptr);

        logarithmicBtn.onClick = [this]() {
            const bool isOn = logarithmicBtn.getToggleState();
            logarithmicBtn.setButtonText (isOn ? "Linear" : "Log");
        };
        logarithmicAttachment = std::make_unique<ButtonPropertyAttachment>(state.displayLogarithmic, logarithmicBtn, nullptr);

        unitBtn.onClick = [this]() {
            const bool isOn = unitBtn.getToggleState();
            unitBtn.setButtonText (isOn ? "Radians" : "Hz");
        };
        unitAttachment = std::make_unique<ButtonPropertyAttachment>(state.displayInHz, unitBtn, nullptr);

        bypassAttachment = std::make_unique<ButtonAttachment>(p.apvts, paramID[PoZeParamID::bypass], bypassBtn);
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

        bounds.removeFromLeft (LAF::Layout::defaultSpacing);
        auto decibelBtnArea = bounds.removeFromLeft (btnSize);
        decibelBtn.setBounds (decibelBtnArea.toNearestInt());

        bounds.removeFromLeft (LAF::Layout::defaultSpacing);
        auto groupDelayArea = bounds.removeFromLeft (btnSize);
        groupDelayBtn.setBounds (groupDelayArea.toNearestInt());

        bounds.removeFromLeft (LAF::Layout::defaultSpacing);
        auto logBtnArea = bounds.removeFromLeft (btnSize);
        logarithmicBtn.setBounds (logBtnArea.toNearestInt());

        bounds.removeFromLeft (LAF::Layout::defaultSpacing);
        auto unitBtnArea = bounds.removeFromLeft (btnSize);
        unitBtn.setBounds (unitBtnArea.toNearestInt());

        bounds.removeFromLeft (LAF::Layout::defaultSpacing);
        auto bypassBtnArea = bounds.removeFromLeft (btnSize);
        bypassBtn.setBounds (bypassBtnArea.toNearestInt());
    }

private:
    State state;
    DragBox gainBox;

    juce::TextButton autoNormalizeBtn       { "Normalize" };
    juce::TextButton decibelBtn             { "Amp" };
    juce::TextButton groupDelayBtn          { "Delay" };
    juce::TextButton logarithmicBtn         { "Linear" };
    juce::TextButton unitBtn                { "Hz" };
    juce::TextButton bypassBtn              { "Bypass" };

    std::unique_ptr<DragBoxAttachment> gainAttachment;
    std::unique_ptr<ButtonAttachment> autoNormalizeAttachment;
    std::unique_ptr<ButtonPropertyAttachment> decibelAttachment;
    std::unique_ptr<ButtonPropertyAttachment> groupDelayAttachment;
    std::unique_ptr<ButtonPropertyAttachment> logarithmicAttachment;
    std::unique_ptr<ButtonPropertyAttachment> unitAttachment;
    std::unique_ptr<ButtonAttachment> bypassAttachment;
};