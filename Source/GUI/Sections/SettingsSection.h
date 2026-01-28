#pragma once

#include <JuceHeader.h>

#include "../../Data/Attachments/DragBoxAttachment.h"
#include "../../Data/State.h"
#include "../Components/DragBox.h"
#include "../LookAndFeel.h"
#include <magic_enum/magic_enum.hpp>

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

        for (auto* button : juce::Array<juce::TextButton*>{&firstPlotBtn, &secondPlotBtn, &decibelBtn, &logarithmicBtn, &unitBtn})
        {
            button->setColour (juce::TextButton::ColourIds::buttonColourId, LAF::Colours::buttonOffColour);
            button->setColour (juce::TextButton::ColourIds::buttonOnColourId, LAF::Colours::buttonOnColour);
            button->setColour (juce::TextButton::ColourIds::textColourOffId, juce::Colours::white);
            button->setColour (juce::TextButton::ColourIds::textColourOnId, juce::Colours::white);
            addAndMakeVisible (button);
        }

        for (auto* button : juce::Array<juce::TextButton*>{&decibelBtn, &logarithmicBtn, &unitBtn, &autoNormalizeBtn, &bypassBtn})
            button->setClickingTogglesState (true);

        for (auto* button : juce::Array<juce::TextButton*>{&autoNormalizeBtn, &bypassBtn})
        {
            button->setColour (juce::TextButton::ColourIds::buttonColourId, LAF::Colours::buttonOffColour);
            button->setColour (juce::TextButton::ColourIds::buttonOnColourId, LAF::Colours::buttonOnColour);
            button->setColour (juce::TextButton::ColourIds::textColourOffId, LAF::Colours::textColour);
            button->setColour (juce::TextButton::ColourIds::textColourOnId, juce::Colours::white);
            addAndMakeVisible (button);
        }

        decibelBtn.onClick = [this]() {
            const bool isOn = decibelBtn.getToggleState();
            decibelBtn.setButtonText (isOn ? "dB" : "Amp");
        };

        firstPlotBtn.onClick = [this]() {
            juce::PopupMenu menu = createPlotMenu (state.firstPlotType.getValue());
            menu.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (firstPlotBtn), [this](int result) {
                if (result > 0)
                    state.firstPlotType.setValue (static_cast<PlotType>(result - 1));
            });
        };;

        secondPlotBtn.onClick = [this]() {
            juce::PopupMenu menu = createPlotMenu (state.secondPlotType.getValue());
            menu.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (secondPlotBtn), [this](int result) {
                if (result > 0)
                    state.secondPlotType.setValue (static_cast<PlotType>(result - 1));
            });
        };;

        logarithmicBtn.onClick = [this]() {
            const bool isOn = logarithmicBtn.getToggleState();
            logarithmicBtn.setButtonText (isOn ? "Linear" : "Log");
        };

        unitBtn.onClick = [this]() {
            const bool isOn = unitBtn.getToggleState();
            unitBtn.setButtonText (isOn ? "Radians" : "Hz");
        };

        autoNormalizeAttachment = std::make_unique<ButtonAttachment>(p.apvts, paramID[PoZeParamID::autoNormalise], autoNormalizeBtn);
        decibelAttachment = std::make_unique<ButtonPropertyAttachment>(state.displayInDB, decibelBtn, nullptr);
        logarithmicAttachment = std::make_unique<ButtonPropertyAttachment>(state.displayLogarithmic, logarithmicBtn, nullptr);
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

        static constexpr int numButtons = 8;
        constexpr float spacingWidth = (float)(numButtons - 1) * LAF::Layout::defaultSpacing;
        const float btnSize = (bounds.getWidth() - spacingWidth) / numButtons;

        auto gainBoxArea = bounds.removeFromLeft (btnSize);
        gainBox.setBounds (gainBoxArea.toNearestInt());

        for (auto* b : juce::Array<juce::Button*>{&autoNormalizeBtn, &firstPlotBtn, &secondPlotBtn, &decibelBtn, &logarithmicBtn, &unitBtn, &bypassBtn})
        {
            bounds.removeFromLeft (LAF::Layout::defaultSpacing);
            auto area = bounds.removeFromLeft (btnSize);
            b->setBounds (area.toNearestInt());
        }
    }

private:

    juce::PopupMenu createPlotMenu(PlotType currentlySelectedPlot)
    {
        juce::PopupMenu menu;
        static constexpr int numPlots = (int)magic_enum::enum_count<PlotType>();

        for (int i = 0; i < numPlots; i++)
        {
            const auto type = static_cast<PlotType>(i);
            juce::String name = magic_enum::enum_name(type).data();
            name = Utils::insertSpacesForEveryUpperCase (name);
            const bool isTicked = type == currentlySelectedPlot;
            menu.addItem (i + 1, name, true, isTicked);
        }

        return menu;
    }

    State state;
    DragBox gainBox;

    juce::TextButton autoNormalizeBtn       { "Normalize" };
    juce::TextButton firstPlotBtn           { "1st plot" };
    juce::TextButton secondPlotBtn           { "2nd plot" };
    juce::TextButton decibelBtn             { "Amp" };
    juce::TextButton logarithmicBtn         { "Linear" };
    juce::TextButton unitBtn                { "Hz" };
    juce::TextButton bypassBtn              { "Bypass" };

    std::unique_ptr<DragBoxAttachment> gainAttachment;
    std::unique_ptr<ButtonAttachment> autoNormalizeAttachment;
    std::unique_ptr<ButtonPropertyAttachment> decibelAttachment;
    std::unique_ptr<ButtonPropertyAttachment> logarithmicAttachment;
    std::unique_ptr<ButtonPropertyAttachment> unitAttachment;
    std::unique_ptr<ButtonAttachment> bypassAttachment;
};