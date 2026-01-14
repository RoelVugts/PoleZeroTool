#pragma once

#include <JuceHeader.h>

class TreeWrapper : public juce::ValueTree::Listener
{
public:

    //==============================================================================
    using PropertyChangeCallbackFn = std::function<void()>;
    using AnyPropertyChangeCallbackFn = std::function<void (juce::ValueTree&)>;
    using ChildAddCallbackFn = std::function<void (juce::ValueTree&)>;
    using ChildRemovedCallbackFn = std::function<void (juce::ValueTree&, int)>;

    //==============================================================================
    TreeWrapper (const juce::ValueTree& tree_) : tree (tree_) { this->tree.addListener (this); }
    ~TreeWrapper() override { tree.removeListener (this); }

    //==============================================================================
    /** Copy constructor
     *  This copy constructors only copies the underlying ValueTree. It does not copy the listener callbacks.
     */
    TreeWrapper (const TreeWrapper& other) : TreeWrapper (other.tree) {}

    TreeWrapper& operator= (const TreeWrapper& other) = delete;
    TreeWrapper& operator== (const TreeWrapper& other) = delete;
    //==============================================================================
    void setOnPropertyChanged (const juce::Identifier ID, PropertyChangeCallbackFn f, bool callImmediately)
    {
        propertyChangedLambdas[ID].push_back (f);

        if (callImmediately)
            f();
    }

    void setOnAnyPropertyChanged (AnyPropertyChangeCallbackFn f) { anyPropertyChangedLambdas.push_back (f); }

    //==============================================================================
    int getNumPropertyChangedCallbacks() const noexcept { return (int) propertyChangedLambdas.size(); }

    //==============================================================================
    juce::ValueTree& getTree() noexcept { return tree; }
    const juce::ValueTree& getTree() const noexcept { return tree; }

protected:

    void valueTreePropertyChanged (juce::ValueTree& tree_, const juce::Identifier& ID) override
    {
        if (tree == tree_)
        {
            if (auto it = propertyChangedLambdas.find (ID); it != propertyChangedLambdas.end())
            {
                for (PropertyChangeCallbackFn& f : it->second)
                    juce::NullCheckedInvocation::invoke (f);
            }
        }

        for (AnyPropertyChangeCallbackFn& f : anyPropertyChangedLambdas)
            juce::NullCheckedInvocation::invoke (f, tree_);
    }

    juce::ValueTree tree;
    std::map<juce::Identifier, std::vector<PropertyChangeCallbackFn>> propertyChangedLambdas;
    std::vector<AnyPropertyChangeCallbackFn> anyPropertyChangedLambdas;

};