#pragma once

#include <JuceHeader.h>

#include "../../GUI/Components/DragBox.h"

/** Keeps a DragBox and an AudioParameter in sync.*/
class DragBoxAttachment : private DragBox::Listener
{
public:
    //===================================================================
    static std::unique_ptr<DragBoxAttachment>
        makeAttachment (juce::AudioProcessorValueTreeState& apvts, const juce::String& paramIdentifier, DragBox& comp, juce::UndoManager* um = nullptr)
    {
        juce::RangedAudioParameter* param = apvts.getParameter (paramIdentifier);
        jassert (param != nullptr);

        return std::make_unique<DragBoxAttachment> (*param, comp, um);
    }

    //===================================================================
    DragBoxAttachment (juce::RangedAudioParameter& param, DragBox& comp, juce::UndoManager* um = nullptr)
        : attachment (param, [this] (float v) { setValue (v); }), dragBox (comp)
    {
        juce::ignoreUnused (um);

        if (auto* choiceParam = dynamic_cast<juce::AudioParameterChoice*>(&param); choiceParam != nullptr)
        {
            const juce::StringArray& values = choiceParam->getAllValueStrings();
            dragBox.valToTextFn = [values](float v) { int index = static_cast<int>(v); return values[index]; };
            dragBox.textToValFn = [values](const juce::String& t) {
                const int index = values.indexOf (t, true);
                return static_cast<float>(index < 0 ? 0 : index);
            };
        }

        const auto& range = param.getNormalisableRange();
        dragBox.setRange (range);
        dragBox.setDefaultValue (range.convertFrom0to1 (param.getDefaultValue()));
        dragBox.addListener (this);

        attachment.sendInitialUpdate();
    }

    ~DragBoxAttachment() override
    {
        dragBox.removeListener (this);
        dragBox.valToTextFn = nullptr;
        dragBox.textToValFn = nullptr;
    }

private:
    //===================================================================
    void setValue (float value)
    {
        juce::ScopedValueSetter<bool> svs (ignoreCallbacks, true);
        dragBox.setValue (value, true);
    }

    //===================================================================
    void dragStarted (DragBox*) override { attachment.beginGesture(); }
    void dragEnded (DragBox*) override { attachment.endGesture(); }

    void valueChanged (DragBox*) override
    {
        if (! ignoreCallbacks)
        {
            attachment.setValueAsPartOfGesture (dragBox.getValue());
        }
    }

    //===================================================================
    juce::ParameterAttachment attachment;
    DragBox& dragBox;
    bool ignoreCallbacks { false };
};