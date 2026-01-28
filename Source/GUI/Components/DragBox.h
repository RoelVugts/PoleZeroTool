#pragma once

#include "../LookAndFeel.h"

#include <JuceHeader.h>

class DragBox : public juce::Component, private juce::Label::Listener
{
public:

    enum ColourIds
    {
        backgroundColourId      = 0x204101,
        textColourId            = 0x204102,
    };

    using TextToValFn = std::function<float(const juce::String&)>;
    using ValToTextFn = std::function<juce::String(float)>;

    class Listener
    {
    public:
        Listener() = default;
        virtual ~Listener() = default;
        virtual void dragStarted(DragBox*) {}
        virtual void dragEnded(DragBox*) {}
        virtual void valueChanged(DragBox*) {}
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Listener)
    };

    void addListener (Listener* listener) { listeners.add (listener); }
    void removeListener (Listener* listener) { listeners.remove (listener); }

    DragBox()
    {
        setColour (backgroundColourId, juce::Colours::black);
        setColour (textColourId, juce::Colour(200, 200, 200));

        text.setColour(juce::Label::ColourIds::outlineWhenEditingColourId, juce::Colours::transparentBlack);
        text.setColour (juce::Label::ColourIds::textColourId, findColour (textColourId));
        text.setJustificationType(juce::Justification::centred);
        text.setEditable (false, true, false);
        text.setInterceptsMouseClicks (false, false);
        addAndMakeVisible (text);

        text.addListener (this);
    }

    ~DragBox() override
    {
        text.removeListener (this);
    }

    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();
        const float cornerSize = std::min(bounds.getHeight(), bounds.getWidth()) * 0.4f;

        g.setColour (findColour (backgroundColourId));
        g.fillRoundedRectangle(bounds, cornerSize);
    }

    void resized() override
    {
        auto bounds = getLocalBounds().toFloat();
        const float width = bounds.getWidth();
        const float height = bounds.getHeight();

        // Margins
        bounds.removeFromLeft (width * leftMarginRatio);
        bounds.removeFromRight (width * leftMarginRatio);

        text.setBounds(bounds.toNearestInt());
        text.setFont(juce::FontOptions (height * 0.7f));
    }

    void setValue(float newValue, bool sendNotification)
    {
        jassert(newValue >= range.start && newValue <= range.end);

        if (approximatelyEqual (newValue, value))
            return;

        value = newValue;
        juce::String t;

        if (valToTextFn != nullptr) t = valToTextFn(newValue);
        else t = juce::String(value, numDecimalsToDisplay);

        text.setText (t, juce::dontSendNotification);
        repaint();

        if (sendNotification)
            listeners.call([this](Listener& l) { l.valueChanged (this); });
    }

    float getValue() const noexcept { return value; }
    juce::String getText() const noexcept { return text.getText (); }

    void setRange(const juce::NormalisableRange<float>& newRange)
    {
        range = newRange;
        const float newValue = juce::jlimit<float>(range.start, range.end, value);
        setValue(newValue, true);
    }

    void setDefaultValue(float defaultVal)
    {
        defaultValue = defaultVal;
    }

    void setNumDecimalsToDisplay(int numDecimals)
    {
        numDecimalsToDisplay = numDecimals;
    }

    /** Adjust the vertical drag sensitivity
     *
     * @param amount    Value between 0 and 10. Value of 1.0 is regular. Lower values result in lower sensitivity,
     *                  higher values in higher sensitivity.
     */
    void setDragSensitivity(float amount)
    {
        dragSensitivity = 1.0f / juce::jlimit<float>(0.01f, 10.0f, amount);
    }

    juce::NormalisableRange<float> getRange() { return range; }

    TextToValFn textToValFn { nullptr };
    ValToTextFn valToTextFn { nullptr };

protected:

    void mouseEnter(const MouseEvent&) override
    {
        mouseIsOver = true;
        text.setColour (juce::Label::ColourIds::textColourId, juce::Colours::white);
        repaint();
    }

    void mouseExit(const MouseEvent&) override
    {
        mouseIsOver = false;
        text.setColour (juce::Label::ColourIds::textColourId, findColour (textColourId));
        repaint();
    }

    void mouseDown(const juce::MouseEvent&) override
    {
        valueOnMouseDown = value;

        listeners.call([this] (Listener& l) { l.dragStarted (this); });
    }

    void mouseUp(const juce::MouseEvent&) override
    {
        listeners.call([this](Listener& l) { l.dragEnded (this); });
    }

    void mouseDoubleClick(const MouseEvent& e) override
    {
        if (e.mods.isCommandDown())
            setValue (defaultValue, true);
        else
            text.showEditor();
    }

    void mouseDrag(const juce::MouseEvent& e) override
    {
        const float sensitivity = e.mods.isShiftDown() ? dragSensitivity * 100.0f : dragSensitivity;
        static constexpr float numPixelsPerInc = 1000.0f;
        const float normDist = -((float)e.getDistanceFromDragStartY() / (numPixelsPerInc * sensitivity));
        const float newNormVal = juce::jlimit<float>(0.0f, 1.0f, range.convertTo0to1 (valueOnMouseDown) + normDist);
        float newValue = range.convertFrom0to1 (newNormVal);
        newValue = juce::jlimit<float>(range.start, range.end, newValue);
        setValue (newValue, true);
    }

    void mouseWheelMove(const MouseEvent& event, const MouseWheelDetails& wheel) override
    {
        float inc = wheel.deltaY;

        #if JUCE_MAC
        if (event.mods.isShiftDown() && wheel.deltaY == 0 && wheel.deltaX != 0)
            inc = wheel.deltaX;
        #endif

        inc *= dragSensitivity * 0.1f;;

        if (event.mods.isShiftDown())
            inc *= 0.01f;

        const float newValue = std::clamp(value + inc, range.start, range.end);
        setValue(newValue, true);
    }

    void labelTextChanged (Label*) override
    {
        juce::String t = text.getText();
        t = t.trimStart();

        if (textToValFn != nullptr)
            setValue(textToValFn(t), true);
        else
            setValue(juce::jlimit<float>(range.start, range.end, t.getFloatValue()), true);
    }

private:
    float value { 0.0f };
    float defaultValue { 0.0f };
    int numDecimalsToDisplay { 2 };
    juce::Label text;

    juce::NormalisableRange<float> range { 0.0f, 1.0f };
    float valueOnMouseDown { 0.0f };
    float dragSensitivity { 1.0f };
    bool mouseIsOver { false };

    juce::ListenerList<Listener> listeners;

    static constexpr float leftMarginRatio { 0.085f };
    static constexpr float topBottomMarginRatio { 0.25f };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DragBox)
};