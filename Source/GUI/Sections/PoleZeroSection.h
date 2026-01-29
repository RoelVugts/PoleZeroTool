#pragma once

#include <JuceHeader.h>

#include "../../Data/Attachments/PoZePlotAttachment.h"
#include "../Components/PoZeHelpDialog.h"
#include "../Components/PoZePlot.h"
#include "../Components/PoZeTable.h"
#include "../LookAndFeel.h"

class PoleZeroSection : public juce::Component
{
public:

    PoleZeroSection(AudioPluginAudioProcessor& p)
        : poZeTable (p.state.poleZeroState)
    {
        poZePlot.setColour (PoZePlot::ColourIds::backgroundColourId, LAF::Colours::plotBackgroundColour);
        poZePlot.setColour (PoZePlot::ColourIds::unitCircleColourId, LAF::Colours::plotGridColour);
        poZePlot.setRange ({ minPoZePlotRange, maxPoZePlotRange }, { minPoZePlotRange, maxPoZePlotRange });

        #if JUCE_MAC
            poZePlot.setTooltip ("The Pole Zero plot | Right click to modify or see help button for key commands");
        #else
            poZePlot.setTooltip ("The Pole Zero plot | Right click to modify or see help button for key commands");
        #endif
        poZePlotAttachment = std::make_unique<PoZePlotAttachment>(p.state.poleZeroState, poZePlot, &p.undoManager);
        addAndMakeVisible (poZePlot);

        poZeTable.setTooltip ("Editable list of the Poles and Zeros");
        addAndMakeVisible (poZeTable);

        //==================================================================================================
        infoBtn.onClick = [this]() {
            auto dialog = std::make_unique<PoZeHelpDialog>();
            dialog->setSize (dialog->getPreferredWidth(), dialog->getPreferredHeight());
            juce::CallOutBox::launchAsynchronously (std::move(dialog), infoBtn.getScreenBounds(), nullptr);
        };
        infoBtn.setColour (juce::TextButton::ColourIds::buttonColourId, LAF::Colours::buttonColour.withAlpha (0.6f));
        infoBtn.setColour (juce::TextButton::ColourIds::buttonOnColourId, LAF::Colours::buttonOnColour);
        infoBtn.setColour (juce::ComboBox::outlineColourId, LAF::Colours::buttonOutlineColour);
        infoBtn.setColour (juce::TextButton::ColourIds::textColourOffId, LAF::Colours::textColour);
        infoBtn.setColour (juce::TextButton::ColourIds::textColourOnId, LAF::Colours::highlightedColour);
        addAndMakeVisible (infoBtn);
    }

    void resized() override
    {
        auto bounds = getLocalBounds().toFloat();

        const float poZePlotSize = bounds.getWidth();
        auto plotArea = bounds.removeFromTop (poZePlotSize);
        poZePlot.setBounds (plotArea.toNearestInt());

        plotArea = plotArea.reduced (LAF::Layout::defaultSpacing);
        const float infoBtnSize = plotArea.getWidth() * 0.1f;
        auto infoBtnArea = plotArea.removeFromTop (infoBtnSize).removeFromRight (infoBtnSize);
        infoBtn.setBounds (infoBtnArea.toNearestInt());

        bounds.removeFromTop (LAF::Layout::defaultSpacing); // Spacing

        auto tableArea = bounds;
        poZeTable.setBounds (tableArea.toNearestInt());
    }

    PoZePlot poZePlot;
    std::unique_ptr<PoZePlotAttachment> poZePlotAttachment;

    PoZeTable poZeTable;

    juce::TextButton infoBtn { "?", "Show help information for using the Pole / Zero Plot" };
};