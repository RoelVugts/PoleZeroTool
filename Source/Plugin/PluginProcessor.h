#pragma once

#include <JuceHeader.h>

#include "../DSP/Filter.h"
#include "../DSP/FilterDesign.h"
#include "../Data/Attachments/FilterDesignAttachment.h"
#include "../Data/State.h"

#include <choc/containers/choc_SingleReaderMultipleWriterFIFO.h>

#define MAX_CHANNELS 2

//==============================================================================
class AudioPluginAudioProcessor final : public juce::AudioProcessor
{
public:
    //==============================================================================
    AudioPluginAudioProcessor();
    ~AudioPluginAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    using AudioProcessor::processBlock;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    //==============================================================================
    State state { juce::ValueTree(State::IDs::type) };
    juce::UndoManager undoManager;

    //==============================================================================
    std::array<ComplexFilter, MAX_CHANNELS> filter;
    FilterDesign filterDesign;
    FilterDesignAttachment filterDesignAttachment;

    //==============================================================================
    choc::fifo::SingleReaderMultipleWriterFIFO<std::function<void()>> dspFifo;
private:

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioPluginAudioProcessor)
};
