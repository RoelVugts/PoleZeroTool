#pragma once

#include <JuceHeader.h>

// Forward declaration
class PoZeToolLaf;


/** A box with a value which can be adjusted by dragging vertically.
 *  Double clicking also makes it a text editor so the value can be entered manually.
 */
class DragBox : public juce::Component, private juce::Label::Listener, public juce::SettableTooltipClient
{
public:

    using TextToValFn = std::function<float(const juce::String&)>;
    using ValToTextFn = std::function<juce::String(float)>;

    enum ColourIds
    {
        backgroundColourId      = 0x206101,
        outlineColourId         = 0x206202,
        textColourId            = 0x206103,
    };

    //======================================================================
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

    //======================================================================
    DragBox();
    ~DragBox() override;

    //======================================================================
    void paint(juce::Graphics& g) override;
    void resized() override;

    //======================================================================
    void setValue(float newValue, bool sendNotification);
    void setDefaultValue(float defaultVal);
    void setRange(const juce::NormalisableRange<float>& newRange);
    void setNumDecimalsToDisplay(int numDecimals);

    /** Adjust the vertical drag sensitivity
     *
     * @param amount    Value between 0 and 10. Value of 1.0 is regular. Lower values result in lower sensitivity,
     *                  higher values in higher sensitivity.
     */
    void setDragSensitivity(float amount);

    //======================================================================
    float getValue() const noexcept;
    juce::String getText() const noexcept;
    juce::NormalisableRange<float> getRange();

    //======================================================================
    class LookAndFeelMethods
    {
    public:
        virtual ~LookAndFeelMethods() = default;
        virtual void drawDragBoxBackground (juce::Graphics& g, DragBox& p) = 0;
    };

    //======================================================================
    // If the displayed value should not be a number then you can use this to determine
    // what text should be displayed for which value.
    TextToValFn textToValFn { nullptr };

    // If the displayed value should not be a number then you can use this to determine
    // what text should be displayed for which value.
    ValToTextFn valToTextFn { nullptr };

protected:

    //======================================================================
    // Internal
    void mouseEnter(const MouseEvent&) override;
    void mouseExit(const MouseEvent&) override;
    void mouseDown(const juce::MouseEvent&) override;
    void mouseUp(const juce::MouseEvent&) override;
    void mouseDoubleClick(const MouseEvent& e) override;
    void mouseDrag(const juce::MouseEvent& e) override;
    void mouseWheelMove(const MouseEvent& event, const MouseWheelDetails& wheel) override;

    //======================================================================
    void labelTextChanged (Label*) override;

private:
    //======================================================================
    PoZeToolLaf* getCustomLookAndFeel() const;

    //======================================================================
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