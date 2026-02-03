#include "DragBox.h"

#include "../LookAndFeel.h"

DragBox::DragBox()
{
    setColour (backgroundColourId, juce::Colours::black);
    setColour (outlineColourId, juce::Colours::grey);
    setColour (textColourId, juce::Colour(200, 200, 200));

    text.setColour(juce::Label::ColourIds::outlineWhenEditingColourId, juce::Colours::transparentBlack);
    text.setColour (juce::Label::ColourIds::textColourId, findColour (textColourId));
    text.setJustificationType(juce::Justification::centred);
    text.setEditable (false, true, false);
    text.setInterceptsMouseClicks (false, false);
    addAndMakeVisible (text);

    text.addListener (this);
}

DragBox::~DragBox()
{
    text.removeListener (this);
}

void DragBox::paint(juce::Graphics& g)
{
    if (auto* lf = getCustomLookAndFeel())
        lf->drawDragBoxBackground (g, *this);
}

void DragBox::resized()
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

void DragBox::setValue(float newValue, bool sendNotification)
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

float DragBox::getValue() const noexcept { return value; }
juce::String DragBox::getText() const noexcept { return text.getText (); }

void DragBox::setRange(const juce::NormalisableRange<float>& newRange)
{
    range = newRange;
    const float newValue = juce::jlimit<float>(range.start, range.end, value);
    setValue(newValue, true);
}

void DragBox::setDefaultValue(float defaultVal)
{
    defaultValue = defaultVal;
}

void DragBox::setNumDecimalsToDisplay(int numDecimals)
{
    numDecimalsToDisplay = numDecimals;
}


void DragBox::setDragSensitivity(float amount)
{
    dragSensitivity = 1.0f / juce::jlimit<float>(0.01f, 10.0f, amount);
}

juce::NormalisableRange<float> DragBox::getRange() { return range; }

void DragBox::mouseEnter(const MouseEvent&)
{
    mouseIsOver = true;
    text.setColour (juce::Label::ColourIds::textColourId, juce::Colours::white);
    repaint();
}

void DragBox::mouseExit(const MouseEvent&)
{
    mouseIsOver = false;
    text.setColour (juce::Label::ColourIds::textColourId, findColour (textColourId));
    repaint();
}

void DragBox::mouseDown(const juce::MouseEvent&)
{
    valueOnMouseDown = value;

    listeners.call([this] (Listener& l) { l.dragStarted (this); });
}

void DragBox::mouseUp(const juce::MouseEvent&)
{
    listeners.call([this](Listener& l) { l.dragEnded (this); });
}

void DragBox::mouseDoubleClick(const MouseEvent& e)
{
    if (e.mods.isCommandDown())
        setValue (defaultValue, true);
    else
        text.showEditor();
}

void DragBox::mouseDrag(const juce::MouseEvent& e)
{
    const float sensitivity = e.mods.isShiftDown() ? dragSensitivity * 100.0f : dragSensitivity;
    static constexpr float numPixelsPerInc = 1000.0f;
    const float normDist = -((float)e.getDistanceFromDragStartY() / (numPixelsPerInc * sensitivity));
    const float newNormVal = juce::jlimit<float>(0.0f, 1.0f, range.convertTo0to1 (valueOnMouseDown) + normDist);
    float newValue = range.convertFrom0to1 (newNormVal);
    newValue = juce::jlimit<float>(range.start, range.end, newValue);
    setValue (newValue, true);
}

void DragBox::mouseWheelMove(const MouseEvent& event, const MouseWheelDetails& wheel)
{
    float inc = wheel.deltaY;

    #if JUCE_MAC
    if (event.mods.isShiftDown() && approximatelyEqual (wheel.deltaY, 0.0f) && ! approximatelyEqual (wheel.deltaX, 0.0f))
        inc = wheel.deltaX;
    #endif

    inc *= dragSensitivity * 0.1f;;

    if (event.mods.isShiftDown())
        inc *= 0.01f;

    const float newValue = std::clamp(value + inc, range.start, range.end);
    setValue(newValue, true);
}

void DragBox::labelTextChanged (Label*)
{
    juce::String t = text.getText();
    t = t.trimStart();

    if (textToValFn != nullptr)
        setValue(textToValFn(t), true);
    else
        setValue(juce::jlimit<float>(range.start, range.end, t.getFloatValue()), true);
}

PoZeToolLaf* DragBox::getCustomLookAndFeel() const
{
    if (auto* lf = dynamic_cast<PoZeToolLaf*>(&getLookAndFeel()))
        return lf;

    return nullptr;
}
