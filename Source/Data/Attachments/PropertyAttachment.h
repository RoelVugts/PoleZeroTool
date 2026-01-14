
#pragma once

#include <JuceHeader.h>


#include "../Utils/TreePropertyWrapper.h"

//===============================================================================
// PropertyStorage

template<typename T, bool = std::is_trivially_copyable_v<T>>
struct PropertyStorage;

// Use atomic when trivially copyable
template<typename T>
struct PropertyStorage<T, true>
{
    std::atomic<T> value;

    PropertyStorage() = default;
    explicit PropertyStorage(T initial) : value(initial) {}

    void set(T v)
    {
        value.store(v, std::memory_order_relaxed);
    }

    T get() const
    {
        return value.load(std::memory_order_relaxed);
    }
};

// Use lock when not trivially copyable
template<typename T>
struct PropertyStorage<T, false>
{
    mutable std::mutex mutex;
    T value;

    PropertyStorage() = default;
    explicit PropertyStorage(T initial) : value(initial) {}

    void set(const T& v)
    {
        std::lock_guard<std::mutex> lock(mutex);
        value = v;
    }

    T get() const
    {
        std::lock_guard<std::mutex> lock(mutex);
        return value;
    }
};

//===============================================================================
// PropertyAttachment

template<class Type>
class PropertyAttachment : private juce::ValueTree::Listener, private juce::AsyncUpdater {
public:

    PropertyAttachment(TreePropertyWrapper<Type>& p, std::function<void(const Type&)> propertyChangedCallback)
    : property(p)
    , setValue(std::move(propertyChangedCallback))
    {
        property.getTree().addListener (this);
    }

    ~PropertyAttachment() override
    {
        property.getTree().removeListener(this);
    }

    // Triggers the property changed callback
    void sendInitialUpdate()
    {
        propertyValueChanged (property.getValue());
    }

    void setPropertyValue(const Type& value)
    {
        property.setValueExcludingListener (value, this);
    }

private:

    void valueTreePropertyChanged(juce::ValueTree& tree, const juce::Identifier& ID) override
    {
        if (tree == property.getTree())
        {
            if (property.getPropertyID() == ID)
                propertyValueChanged(property.getValue());
        }
    }

    void propertyValueChanged (const Type& newValue)
    {
        lastValue.set(newValue);

        if (MessageManager::getInstance()->isThisTheMessageThread())
        {
            cancelPendingUpdate();
            handleAsyncUpdate();
        }
        else
        {
            triggerAsyncUpdate();
        }
    }

    void handleAsyncUpdate() override
    {
        NullCheckedInvocation::invoke (setValue, lastValue.get());
    }

    TreePropertyWrapper<Type> property;
    PropertyStorage<Type> lastValue {};
    std::function<void(Type)> setValue { nullptr };
};

//========================================================================
// ButtonPropertyAttachment

class ButtonPropertyAttachment : private juce::Button::Listener
{
public:
    ButtonPropertyAttachment (TreePropertyWrapper<bool>& prop, juce::Button& b, juce::UndoManager* um)
        : attachment (prop, [this] (float v) { setValue (v); })
        , button (b)
    {
        juce::ignoreUnused (um);

        sendInitialUpdate();
        button.addListener (this);
    }

    ~ButtonPropertyAttachment() override
    {
        button.removeListener (this);
    }

private:
    void sendInitialUpdate()
    {
        attachment.sendInitialUpdate();
    }

    void setValue (float newValue)
    {
        const ScopedValueSetter<bool> svs (ignoreCallbacks, true);
        button.setToggleState (static_cast<bool> (newValue), sendNotificationSync);
    }

    void buttonStateChanged(Button*) override
    {
        if (! ignoreCallbacks)
            attachment.setPropertyValue (button.getToggleState());
    }

    void buttonClicked(Button*) override
    {
        // Nothing to do here, we only trigger callback whenever the state changes
    }

    PropertyAttachment<bool> attachment;
    juce::Button& button;
    bool ignoreCallbacks { false };
};