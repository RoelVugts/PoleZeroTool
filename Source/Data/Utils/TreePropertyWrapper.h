#pragma once

#include <JuceHeader.h>

/** A wrapper around a ValueTree property. Similar to juce::CachedValue
    but a bit simpler.
 */
template <class T>
class TreePropertyWrapper
{
public:

    //==============================================================================
    TreePropertyWrapper() = default;

    TreePropertyWrapper (const juce::ValueTree& tree_, const juce::Identifier ID_, const T defaultValue_ = {}, juce::UndoManager* um = nullptr)
        : tree (tree_), ID (ID_), defaultValue (defaultValue_), undoManager (um)
    {
        jassert (tree.isValid());
    }

    TreePropertyWrapper (const TreePropertyWrapper& other) : TreePropertyWrapper (other.tree, other.ID, other.defaultValue, other.undoManager) {}

    TreePropertyWrapper& operator= (const TreePropertyWrapper& other)
    {
        if (this == &other)
            return *this;

        tree = other.tree;
        ID = other.ID;
        defaultValue = other.defaultValue;

        return *this;
    }

    //==============================================================================
    void setValue (T value)
    {
        const juce::var var = juce::VariantConverter<T>::toVar (value);
        tree.setProperty (ID, var, undoManager);
    }

    void setValueExcludingListener (T value, juce::ValueTree::Listener* listenerToExclude)
    {
        const juce::var var = juce::VariantConverter<T>::toVar(value);
        tree.setPropertyExcludingListener (listenerToExclude, ID, var, undoManager);
    }

    //==============================================================================
    const juce::Identifier getPropertyID() const { return ID; }

    T getValue() const
    {
        T value = defaultValue;
        if (tree.hasProperty (ID))
            value = juce::VariantConverter<T>::fromVar (getVar());
        return value;
    }

    T getDefaultValue() const { return defaultValue; }

    juce::var getVar() const { return tree.getProperty (ID); }
    juce::ValueTree& getTree() noexcept { return tree; }

private:
    juce::ValueTree tree;
    juce::Identifier ID;
    T defaultValue = {};
    juce::UndoManager* undoManager { nullptr };
};