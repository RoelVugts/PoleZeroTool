#include "PluginProcessor.h"

#include "PluginEditor.h"

#include "../DSP/MathFunctions.h"

//==============================================================================
AudioPluginAudioProcessor::AudioPluginAudioProcessor()
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ), apvts(*this, nullptr, APVTS_ID, createParameters())
{
    // Create listeners for all parameters
    for (int i = 0; i < apvts.state.getNumChildren(); i++)
        if (const auto child = apvts.state.getChild (i); child.isValid())
            apvts.addParameterListener(child.getProperty("id").toString(), this);

    auto* param = apvts.getParameter (getParamID(PoZeParamID::Gain));
    jassert(param != nullptr);

    filterDesignAttachment = std::make_unique<FilterDesignAttachment>(state.poleZeroState, filterDesign, *param);
}

AudioPluginAudioProcessor::~AudioPluginAudioProcessor()
{

}

//==============================================================================
const juce::String AudioPluginAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool AudioPluginAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool AudioPluginAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool AudioPluginAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double AudioPluginAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int AudioPluginAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int AudioPluginAudioProcessor::getCurrentProgram()
{
    return 0;
}

void AudioPluginAudioProcessor::setCurrentProgram (int index)
{
    juce::ignoreUnused (index);
}

const juce::String AudioPluginAudioProcessor::getProgramName (int index)
{
    juce::ignoreUnused (index);
    return {};
}

void AudioPluginAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
    juce::ignoreUnused (index, newName);
}

//==============================================================================
void AudioPluginAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    juce::ignoreUnused (sampleRate, samplesPerBlock);

    juce::NullCheckedInvocation::invoke(onSampleRateChange, sampleRate);
}

void AudioPluginAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

bool AudioPluginAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}

void AudioPluginAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                              juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused (midiMessages);

    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    if (filterDesignAttachment->newCoefsReady())
    {
        const auto& coefs = filterDesignAttachment->getCoefficients();
        for (auto& f : filter)
            f.setCoefficients (coefs.iirCoefs, coefs.firCoefs);
    }

    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer (channel);

        inputLevel[channel].store(buffer.getRMSLevel (channel, 0, buffer.getNumSamples()), std::memory_order_relaxed);

        double realSum = 0.0;
        double imagSum = 0.0;

        for (int sample = 0; sample < buffer.getNumSamples(); sample++)
        {
            auto processed = bypassed.load(std::memory_order_relaxed) ? std::complex<float>(channelData[sample], 0.0f)
                                                                                   : filter[(size_t)channel].processSample (channelData[sample]);

            channelData[sample] = processed.real() + processed.imag();
            realSum += (double)(processed.real() * processed.real());
            imagSum += (double)(processed.imag() * processed.imag());
        }

        outputLevel[channel].store((float)std::sqrt(realSum / (double)buffer.getNumSamples()), std::memory_order_relaxed);
        outputLevelImag[channel].store((float)std::sqrt(imagSum / (double)buffer.getNumSamples()), std::memory_order_relaxed);
    }
}

//==============================================================================
bool AudioPluginAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* AudioPluginAudioProcessor::createEditor()
{
    return new AudioPluginAudioProcessorEditor (*this);
}

//==============================================================================
void AudioPluginAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    juce::ValueTree tree { "StateInformation" };
    juce::ValueTree data = state.getTree().createCopy();
    juce::ValueTree params = apvts.copyState();
    tree.appendChild (data, nullptr);
    tree.appendChild (params, nullptr);

    auto xml = tree.createXml();
    copyXmlToBinary (*xml, destData);
}

void AudioPluginAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    auto xml = getXmlFromBinary (data, sizeInBytes);
    if (xml != nullptr)
    {
        auto tree = juce::ValueTree::fromXml (*xml);

        if (tree.hasType ("StateInformation"))
        {
            if (auto child = tree.getChildWithName (State::IDs::type); child.isValid())
                state.setState(child);

            if (auto child = tree.getChildWithName (APVTS_ID); child.isValid())
                apvts.replaceState (child);
        }
    }
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AudioPluginAudioProcessor();
}

juce::AudioProcessorValueTreeState::ParameterLayout AudioPluginAudioProcessor::createParameters()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    for (int i = 0; i < (int)magic_enum::enum_count<PoZeParamID>(); i++)
    {
        const auto id = static_cast<PoZeParamID>(i);
        const juce::String& paramID = getParamID(id);
        const juce::String& paramName = getParamName(id);

        switch (id)
        {
            case PoZeParamID::Gain:
                params.push_back (std::make_unique<juce::AudioParameterFloat>(juce::ParameterID(paramID, 1), paramName, juce::NormalisableRange<float>{ 0.0f, 1.0f, 1e-7f }, 1.0f));
                break;

            case PoZeParamID::AutoNormalise:
                params.push_back (std::make_unique<juce::AudioParameterBool>(juce::ParameterID(paramID, 1), paramName, true));
                break;

            case PoZeParamID::Bypass:
                params.push_back (std::make_unique<juce::AudioParameterBool>(juce::ParameterID(paramID, 1), paramName, false));
                break;

            default: jassertfalse; break;
        }
    }

    return { params.begin(), params.end() };
}

juce::AudioProcessorParameter* AudioPluginAudioProcessor::getBypassParameter() const
{
    return apvts.getParameter (getParamID(PoZeParamID::Bypass));
}

void AudioPluginAudioProcessor::parameterChanged (const juce::String& parameterID, float newValue)
{
    const auto paramID = magic_enum::enum_cast<PoZeParamID>(parameterID.toStdString(), magic_enum::case_insensitive);

    if (paramID.has_value())
    {
        switch (paramID.value())
        {
            case PoZeParamID::Gain:
                filterDesign.setGain ((double)newValue);
                break;

            case PoZeParamID::AutoNormalise:
                filterDesign.setAutoNormalize (static_cast<bool> (newValue));
                break;

            case PoZeParamID::Bypass:
                bypassed.store(static_cast<bool>(newValue), std::memory_order_relaxed);
                break;

            default: jassertfalse; break;
        }
    }
}
