#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor (AudioPluginAudioProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p), gui(p)
{
    juce::ignoreUnused (processorRef);

    juce::LookAndFeel::setDefaultLookAndFeel (&laf);

    static constexpr int defaultWidth = 825;
    static constexpr int defaultHeight = 550;

    setSize (defaultWidth, defaultHeight);
    setResizable (true, true);
    constrainer.setMinimumSize (defaultWidth / 2, defaultHeight / 2);
    constrainer.setFixedAspectRatio ((double)defaultWidth / (double)defaultHeight);
    setConstrainer (&constrainer);

    addAndMakeVisible (gui);
}

AudioPluginAudioProcessorEditor::~AudioPluginAudioProcessorEditor()
{
}

//==============================================================================
void AudioPluginAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
}

void AudioPluginAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..

    gui.setBounds (getLocalBounds());
}
