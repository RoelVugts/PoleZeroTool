#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor (AudioPluginAudioProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p), gui(p)
{
    juce::ignoreUnused (processorRef);

    //========================================================
    juce::LookAndFeel::setDefaultLookAndFeel (&laf);

    if (auto laf4 = dynamic_cast<juce::LookAndFeel_V4*> (&getLookAndFeel()))
    {
        auto& colourScheme = laf4->getCurrentColourScheme();
        colourScheme.setUIColour (juce::LookAndFeel_V4::ColourScheme::widgetBackground,
                                  LAF::Colours::darkBackgroundColour);
    }

    //========================================================
    static constexpr int defaultWidth = 825;
    static constexpr int defaultHeight = 550;

    //========================================================
    setSize (defaultWidth, defaultHeight);
    setResizable (true, true);
    constrainer.setMinimumSize (defaultWidth, defaultHeight);
    constrainer.setFixedAspectRatio ((double)defaultWidth / (double)defaultHeight);
    setConstrainer (&constrainer);

    //========================================================
    addAndMakeVisible (gui);
}

void AudioPluginAudioProcessorEditor::resized()
{
    gui.setBounds (getLocalBounds());
}
