#include "PoZePlot.h"

#include "../LookAndFeel.h"
PoZePlot::Point::Point (Type type_) : type(type_)
{
}

//======================================================================

void PoZePlot::Point::setValue (float x_, float y_, bool sendNotification)
{
    // Out of range !
    jassert(x_ >= xRange.start && x_ <= xRange.end);
    jassert(y_ >= yRange.start && y_ <= yRange.end);

    x = x_;
    y = y_;
    updatePosition();

    if (sendNotification)
        listeners.call([this](Listener& l) { l.pointValueChanged (this); });
}

void PoZePlot::Point::setNormalizedValue(float x_, float y_, bool sendNotification)
{
    // Expects normalized values !
    jassert(x_ >= 0.0f && x_ <= 1.0f);
    jassert(y_ >= 0.0f && y_ <= 1.0f);

    setValue (xRange.convertFrom0to1 (x_), yRange.convertFrom0to1 (y_), sendNotification);
}

void PoZePlot::Point::setRange (const juce::NormalisableRange<float>& xRange_, const juce::NormalisableRange<float>& yRange_)
{
    xRange = xRange_;
    yRange = yRange_;
    updatePosition();
}


juce::Point<float> PoZePlot::Point::getValue() const noexcept
{
    return { getXValue(), getYValue() };
}

float PoZePlot::Point::getXValue() const noexcept
{
    return x;
}

float PoZePlot::Point::getYValue() const noexcept
{
    return y;
}

void PoZePlot::Point::setType (Type type_)
{
    type = type_;
    repaint();
}

//======================================================================

void PoZePlot::Point::paintWithinCorners(juce::Graphics& g)
{
    if (auto* lf = dynamic_cast<PozeToolLaf*>(&getLookAndFeel()))
        lf->drawPoZePoint (g, *this, mouseIsOver);
}

void PoZePlot::Point::resized()
{
    auto bounds = getLocalBounds();

    // Don't allow to go off-screen
    constrainer.setMinimumOnscreenAmounts (bounds.getHeight(), bounds.getWidth(), bounds.getHeight(), bounds.getWidth());
    updatePosition();
}
//======================================================================

void PoZePlot::Point::updatePosition()
{
    if (auto* parent = getParentComponent())
    {
        const auto parentBounds = parent->getBounds().toFloat();
        const int newX = (int)std::round(parentBounds.getWidth() * xRange.convertTo0to1 (x));
        const int newY = (int)std::round(parentBounds.getHeight() * (1.0f - yRange.convertTo0to1 (y)));
        setCentrePosition(newX, newY);
    }
}

//======================================================================

void PoZePlot::Point::mouseDown(const juce::MouseEvent& event)
{
    setMouseCursor (juce::MouseCursor::NoCursor);
    dragger.startDraggingComponent(this, event);

    listeners.call([&](PoZePlot::Point::Listener& l) { l.clickedOnPoint(this, event.mods.isAltDown()); });
}

void PoZePlot::Point::mouseDrag(const juce::MouseEvent& event)
{
    dragger.dragComponent(this, event, &constrainer);

    if (auto* parent = getParentComponent())
    {
        const auto parentBounds = parent->getBounds().toFloat();
        x = xRange.convertFrom0to1 ((float)getBounds().getCentreX() / parentBounds.getWidth());
        y = yRange.convertFrom0to1 (1.0f - (float)getBounds().getCentreY() / parentBounds.getHeight());
    }

    listeners.call([&](PoZePlot::Point::Listener& l) { l.pointValueChanged(this); });
}

void PoZePlot::Point::mouseUp(const juce::MouseEvent& event)
{
    juce::ignoreUnused (event);
    setMouseCursor (juce::MouseCursor::NormalCursor);
}

void PoZePlot::Point::mouseDoubleClick(const juce::MouseEvent& event)
{
    juce::ignoreUnused (event);
    listeners.call([&](PoZePlot::Point::Listener& l) { l.doubleClickedOnPoint(this); });
}

void PoZePlot::Point::mouseEnter (const juce::MouseEvent& event)
{
    mouseIsOver = true;
    repaint();
}

void PoZePlot::Point::mouseExit (const juce::MouseEvent& event)
{
    mouseIsOver = false;
    setMouseCursor (juce::MouseCursor::NormalCursor);
    repaint();
}

//======================================================================
//PoZePlot

PoZePlot::PoZePlot()
    : xRange(-1.5f, 1.5f), yRange (-1.5f, 1.5f)
{
    setColour (backgroundColourId, juce::Colours::black);
}

PoZePlot::Point* PoZePlot::addPoint (PoZePlot::Point::Type type, float x, float y, bool sendNotification)
{
    auto* point = points.add(std::make_unique<PoZePlot::Point>(type));

    point->setRange (xRange, yRange);
    point->setValue (x, y, true);
    addAndMakeVisible (point);
    resized();

    point->addListener (&pointListener);

    if (sendNotification)
        listeners.call([this](Listener& l) { l.pointAdded (this, points.size() - 1); });

    return point;
}

void PoZePlot::removePoint (int index, bool sendNotification)
{
    jassert(isPositiveAndBelow (index, points.size()));

    points.remove (index);

    if (sendNotification)
        listeners.call([this, index](Listener& l) { l.pointRemoved (this, index); });
}

void PoZePlot::removeAllPoints (bool sendNotification)
{
    while (! points.isEmpty())
        removePoint (points.size() - 1, sendNotification);
}


void PoZePlot::setRange (const juce::NormalisableRange<float>& xRange_, const juce::NormalisableRange<float>& yRange_)
{
    xRange = xRange_;
    yRange = yRange_;

    for (auto* point : points)
        point->setRange (xRange_, yRange_);
}

PoZePlot::Point* PoZePlot::getPoint (int index)
{
    jassert(isPositiveAndBelow (index, points.size()));

    return points.getUnchecked (index);
}



void PoZePlot::paintWithinCorners(juce::Graphics& g)
{
    g.fillAll (juce::Colours::black);

    const auto bounds = getLocalBounds().toFloat();

    // Draw unit circle
    g.setColour(juce::Colours::white);
    g.drawEllipse(unitCircleArea, 1.0f);

    // Draw axis lines
    g.drawLine(bounds.getWidth() * 0.05f,bounds.getHeight() * 0.5f, bounds.getWidth() * 0.95f, bounds.getHeight() * 0.5f);
    g.drawLine(bounds.getWidth() / 2.0f, bounds.getHeight() * 0.05f, bounds.getWidth() / 2.0f, bounds.getHeight() * 0.95f);
}

void PoZePlot::resized()
{
    const auto bounds = getLocalBounds().toFloat();
    const float cornerSize = (float)std::min(getWidth(), getHeight()) * 0.08f;
    setRoundedCorners (cornerSize);

    const int pointSize = (int)std::min(bounds.getWidth(), bounds.getHeight()) * 0.05f;

    for (auto* point : points)
    {
        point->setSize (pointSize, pointSize);
        point->updatePosition();
    }

    unitCircleArea.setX (xRange.convertTo0to1 (-1.0f) * bounds.getWidth());
    unitCircleArea.setRight (xRange.convertTo0to1 (1.0f) * bounds.getWidth());
    unitCircleArea.setTop (yRange.convertTo0to1 (-1.0f) * bounds.getHeight());
    unitCircleArea.setBottom (yRange.convertTo0to1 (1.0f) * bounds.getHeight());
}

//======================================================================
void PoZePlot::mouseDown (const juce::MouseEvent& event)
{
    const float x = event.position.x / (float)getWidth();
    const float y = 1.0f - event.position.y / (float)getHeight();

    if (event.mods.isCommandDown())
        addPoint (PoZePlot::Point::Type::pole, xRange.convertFrom0to1 (x), yRange.convertFrom0to1 (y), true);
    else if (event.mods.isShiftDown())
        addPoint (PoZePlot::Point::Type::zero, xRange.convertFrom0to1 (x), yRange.convertFrom0to1 (y), true);
}

//======================================================================
void PoZePlot::PointListener::clickedOnPoint(PoZePlot::Point* point, bool altClick)
{
    owner.clickedOnPoint(point, altClick);
}

void PoZePlot::clickedOnPoint(PoZePlot::Point* point, bool altClick)
{
    if (altClick)
        points.removeObject (point);
}