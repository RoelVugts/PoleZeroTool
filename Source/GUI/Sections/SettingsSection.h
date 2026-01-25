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

        autoNormalizeBtn.setColour (juce::TextButton::ColourIds::buttonColourId, LAF::Colours::buttonOffColour);
        autoNormalizeBtn.setColour (juce::TextButton::ColourIds::buttonOnColourId, LAF::Colours::buttonOnColour);
        autoNormalizeBtn.setColour (juce::TextButton::ColourIds::textColourOffId, LAF::Colours::textColour);
        autoNormalizeBtn.setColour (juce::TextButton::ColourIds::textColourOnId, juce::Colours::white);
        autoNormalizeBtn.setClickingTogglesState (true);
        autoNormalizeAttachment = std::make_unique<ButtonAttachment>(p.apvts, paramID[PoZeParamID::autoNormalise], autoNormalizeBtn);
        addAndMakeVisible (autoNormalizeBtn);

        decibelBtn.setColour (juce::TextButton::ColourIds::buttonColourId, LAF::Colours::buttonOnColour);
        decibelBtn.setColour (juce::TextButton::ColourIds::buttonOnColourId, LAF::Colours::buttonOnColour);
        decibelBtn.setColour (juce::TextButton::ColourIds::textColourOffId, juce::Colours::white);
        decibelBtn.setColour (juce::TextButton::ColourIds::textColourOnId, juce::Colours::white);
        decibelBtn.setClickingTogglesState (true);
        decibelBtn.onStateChange = [this]() {
            const bool isOn = decibelBtn.getToggleState();
            decibelBtn.setButtonText (isOn ? "dB" : "Amp");
        };
        decibelAttachment = std::make_unique<ButtonPropertyAttachment>(state.displayInDB, decibelBtn, nullptr);
        addAndMakeVisible (decibelBtn);

        groupDelayBtn.setColour (juce::TextButton::ColourIds::buttonColourId, LAF::Colours::buttonOnColour);
        groupDelayBtn.setColour (juce::TextButton::ColourIds::buttonOnColourId, LAF::Colours::buttonOnColour);
        groupDelayBtn.setColour (juce::TextButton::ColourIds::textColourOffId, juce::Colours::white);
        groupDelayBtn.setColour (juce::TextButton::ColourIds::textColourOnId, juce::Colours::white);
        groupDelayBtn.setClickingTogglesState (true);
        groupDelayBtn.onStateChange = [this]() {
            const bool isOn = groupDelayBtn.getToggleState();
            groupDelayBtn.setButtonText (isOn ? "Delay" : "Phase");
        };
        groupDelayAttachment = std::make_unique<ButtonPropertyAttachment>(state.displayGroupDelay, groupDelayBtn, nullptr);
        addAndMakeVisible (groupDelayBtn);

        logarithmicBtn.setColour (juce::TextButton::ColourIds::buttonColourId, LAF::Colours::buttonOnColour);
        logarithmicBtn.setColour (juce::TextButton::ColourIds::buttonOnColourId, LAF::Colours::buttonOnColour);
        logarithmicBtn.setColour (juce::TextButton::ColourIds::textColourOffId, juce::Colours::white);
        logarithmicBtn.setColour (juce::TextButton::ColourIds::textColourOnId, juce::Colours::white);
        logarithmicBtn.setClickingTogglesState (true);
        logarithmicBtn.onStateChange = [this]() {
            const bool isOn = logarithmicBtn.getToggleState();
            logarithmicBtn.setButtonText (isOn ? "Log" : "Linear");
        };
        logarithmicAttachment = std::make_unique<ButtonPropertyAttachment>(state.displayLogarithmic, logarithmicBtn, nullptr);
        addAndMakeVisible (logarithmicBtn);
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
    }

private:
    State state;
    DragBox gainBox;
    juce::TextButton autoNormalizeBtn { "Normalize" };
    juce::TextButton decibelBtn { "Amp" };
    juce::TextButton groupDelayBtn { "Delay" };
    juce::TextButton logarithmicBtn { "Linear" };
    std::unique_ptr<DragBoxAttachment> gainAttachment;
    std::unique_ptr<ButtonAttachment> autoNormalizeAttachment;
    std::unique_ptr<ButtonPropertyAttachment> decibelAttachment;
    std::unique_ptr<ButtonPropertyAttachment> groupDelayAttachment;
    std::unique_ptr<ButtonPropertyAttachment> logarithmicAttachment;
};