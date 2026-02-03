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
        gainBox.setColour (DragBox::ColourIds::backgroundColourId, LAF::Colours::buttonColour);
        gainBox.setColour (DragBox::ColourIds::textColourId, LAF::Colours::textColour);
        gainBox.setColour (DragBox::ColourIds::outlineColourId, LAF::Colours::buttonOutlineColour);
        gainAttachment = DragBoxAttachment::makeAttachment (p.apvts, getParamID(PoZeParamID::Gain), gainBox);
        addAndMakeVisible (gainBox);

        for (auto* button : juce::Array<juce::TextButton*>{&firstPlotBtn, &secondPlotBtn, &decibelBtn, &logarithmicBtn, &unitBtn})
        {
            button->setColour (juce::TextButton::ColourIds::buttonColourId, LAF::Colours::buttonColour);
            button->setColour (juce::TextButton::ColourIds::buttonOnColourId, LAF::Colours::buttonColour);
            button->setColour (juce::ComboBox::outlineColourId, LAF::Colours::buttonOutlineColour);
            button->setColour (juce::TextButton::ColourIds::textColourOffId, LAF::Colours::textColour);
            button->setColour (juce::TextButton::ColourIds::textColourOnId, LAF::Colours::textColour);
            addAndMakeVisible (button);
        }

        for (auto* button : juce::Array<juce::TextButton*>{&decibelBtn, &logarithmicBtn, &unitBtn, &autoNormalizeBtn, &bypassBtn})
            button->setClickingTogglesState (true);

        for (auto* button : juce::Array<juce::TextButton*>{&autoNormalizeBtn, &bypassBtn})
        {
            button->setColour (juce::TextButton::ColourIds::buttonColourId, LAF::Colours::buttonColour);
            button->setColour (juce::TextButton::ColourIds::buttonOnColourId, LAF::Colours::buttonOnColour);
            button->setColour (juce::ComboBox::outlineColourId, LAF::Colours::buttonOutlineColour);
            button->setColour (juce::TextButton::ColourIds::textColourOffId, LAF::Colours::disabledTextColour);
            button->setColour (juce::TextButton::ColourIds::textColourOnId, LAF::Colours::textColour);
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

        autoNormalizeAttachment = std::make_unique<ButtonAttachment>(p.apvts, getParamID(PoZeParamID::AutoNormalise), autoNormalizeBtn);
        decibelAttachment = std::make_unique<ButtonPropertyAttachment>(state.displayInDB, decibelBtn, nullptr);
        logarithmicAttachment = std::make_unique<ButtonPropertyAttachment>(state.displayLogarithmic, logarithmicBtn, nullptr);
        unitAttachment = std::make_unique<ButtonPropertyAttachment>(state.displayInHz, unitBtn, nullptr);
        bypassAttachment = std::make_unique<ButtonAttachment>(p.apvts, getParamID(PoZeParamID::Bypass), bypassBtn);

        gainBox.setTooltip ("The gain applied to the filter");
        autoNormalizeBtn.setTooltip ("If on, will set the gain so the higest peak of the filter is at unity gain");
        firstPlotBtn.setTooltip ("Set the plot type for the first plot");
        secondPlotBtn.setTooltip ("Set the plot type for the second plot");
        decibelBtn.setTooltip ("Display the magnitude plot in decibels or in amplitude");
        logarithmicBtn.setTooltip ("Display the domain (x-axis) logarithmically or linear");
        unitBtn.setTooltip ("Display the x-axis in radians (fractions of pi) or frequency (Hz)");
        bypassBtn.setTooltip ("Bypass the filter");
    }


    void paint(Graphics& g) override
    {
        g.setColour (LAF::Colours::secondaryColour);
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