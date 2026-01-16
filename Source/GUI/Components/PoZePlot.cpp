#include "PoZePlot.h"

#include "../LookAndFeel.h"

const int PoZePlot::Point::angleKeyCode { juce::KeyPress::createFromDescription ("r").getKeyCode() };
const int PoZePlot::Point::magKeyCode { juce::KeyPress::createFromDescription ("m").getKeyCode() };

PoZePlot::Point::Point (Type type_) : type(type_)
{
    setWantsKeyboardFocus (true);
}

//======================================================================

void PoZePlot::Point::setValue (float x_, float y_, bool sendNotification)
{
    // Out of range !
    jassert(x_ >= xRange.start && x_ <= xRange.end);
    jassert(y_ >= yRange.start && y_ <= yRange.end);

    // Prevent infinite loop when updating conjugate
    if (x == x_ && y == y_)
        return;


    x = x_;
    y = y_;
    updatePosition();

    if (sendNotification)
        listeners.call([this](Listener& l) { l.pointValueChanged (this); });

    if (conjugate != nullptr)
        conjugate->setValue (x, -y, true);
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

float PoZePlot::Point::getAngle() const noexcept
{
    return std::atan2(getYValue(), getXValue());
}


void PoZePlot::Point::setType (Type type_)
{
    type = type_;
    repaint();
}

void PoZePlot::Point::setConjugate (Point* conjugatePont)
{
    conjugate = conjugatePont;
}

//======================================================================

void PoZePlot::Point::paintWithinCorners(juce::Graphics& g)
{
    if (auto* lf = getCustomLookAndFeel())
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

    if (dragMode == DragMode::normal)
        dragger.startDraggingComponent(this, event);


    valueOnMouseDown = getValue();
    if (auto* parent = getParentComponent())
        posOnMouseDown = parent->getLocalPoint (this, event.position);

    listeners.call([&](PoZePlot::Point::Listener& l) { l.clickedOnPoint(this, event); });
}

void PoZePlot::Point::mouseDrag(const juce::MouseEvent& event)
{


    if (auto* parent = getParentComponent())
    {
        const auto parentBounds = parent->getLocalBounds().toFloat();

        switch (dragMode)
        {
            case DragMode::normal:
            {
                dragger.dragComponent(this, event, &constrainer);
                const float xNorm = (float)getBounds().getCentreX() / parentBounds.getWidth();
                const float yNorm = 1.0f - ((float)getBounds().getCentreY() / parentBounds.getHeight());
                setNormalizedValue (xNorm, yNorm, true);
                break;
            }

            case DragMode::angle:
            {
                // Get unit circle center (origin)
                const auto origin = parentBounds.getCentre();

                // Get angle from origin
                const auto point = parent->getMouseXYRelative().toFloat() - origin;

                const float newAngle = -std::atan2(point.y, point.x);
                const float mag = valueOnMouseDown.getDistanceFromOrigin();
                setValue (mag * std::cos(newAngle), mag * std::sin(newAngle), true);
                break;
            }

            case DragMode::magnitude:
            {
                const float angleOnMouseDown = std::atan2(valueOnMouseDown.y, valueOnMouseDown.x);
                const juce::Point<float> dir (std::cos(angleOnMouseDown), std::sin(angleOnMouseDown));

                // Mouse position in value space
                const auto point = parent->getMouseXYRelative().toFloat();
                const float xNorm = std::clamp(point.x / parentBounds.getWidth(), 0.0f, 1.0f);
                const float yNorm = std::clamp(1.0f - (point.y / parentBounds.getHeight()), 0.0f, 1.0f);
                const juce::Point<float> mouseValue (xRange.convertFrom0to1(xNorm),yRange.convertFrom0to1(yNorm));

                float magnitude = mouseValue.x * dir.x + mouseValue.y * dir.y;

                // Constrain magnitude
                const float maxMag = std::min(xRange.end, yRange.end);
                magnitude = std::clamp(magnitude, 0.0f, maxMag);

                setValue(magnitude * dir.x,magnitude * dir.y, true);
                break;
            }
        }
    }
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

bool PoZePlot::Point::keyPressed (const juce::KeyPress& key, juce::Component* comp)
{
    if (key.getKeyCode() == angleKeyCode && dragMode != DragMode::angle)
    {
        dragMode = DragMode::angle;

        if (auto* lf = getCustomLookAndFeel())
            setMouseCursor (LAF::Cursors::rotateCursor);
    }
    else if (key.getKeyCode() == magKeyCode && dragMode != DragMode::magnitude)
    {
        std::cout << "Cursor" << std::endl;
        dragMode = DragMode::magnitude;

        setMouseCursor (getRotatedMagnitudeCursor (getAngle()));
    }
    return true;
}

bool PoZePlot::Point::keyStateChanged (bool isKeyDown, Component* originatingComponent)
{
    if (! isKeyDown)
    {
        if (juce::KeyPress::isKeyCurrentlyDown (angleKeyCode))
        {
            if (auto* lf = getCustomLookAndFeel())
                setMouseCursor (LAF::Cursors::rotateCursor);

            dragMode = DragMode::angle;
        }
        else if (juce::KeyPress::isKeyCurrentlyDown (magKeyCode))
        {
            setMouseCursor (getRotatedMagnitudeCursor (getAngle()));

            dragMode = DragMode::magnitude;
        }
        else
        {
            setMouseCursor (juce::MouseCursor::NormalCursor);
            dragMode = DragMode::normal;
        }
    }

    return true;
}


PoZeToolLaf* PoZePlot::Point::getCustomLookAndFeel() const
{
    if (auto* lf = dynamic_cast<PoZeToolLaf*>(&getLookAndFeel()))
        return lf;

    return nullptr;
}

juce::MouseCursor PoZePlot::Point::getRotatedMagnitudeCursor (const float angle)
{

    auto svg = juce::Drawable::createFromImageData (
     BinaryData::MagCursor_png,
     BinaryData::MagCursor_pngSize
    );

    jassert (svg != nullptr);

    auto bounds = svg->getDrawableBounds();

    // Make size big enough for rotation
    const float maxDim = std::max (bounds.getWidth(), bounds.getHeight());

    // Max size is at 45 degrees, with size = side * sqrt(2)
    const int imageSize = juce::roundToInt (std::sqrt (2.0f) * maxDim);

    juce::Image image (juce::Image::ARGB, imageSize, imageSize, true);

    {
        juce::Graphics g (image);

        // Move SVG to center of image
        auto transform =
            juce::AffineTransform::translation (
                imageSize * 0.5f - bounds.getCentreX(),
                imageSize * 0.5f - bounds.getCentreY()
            )
            .rotated (-angle,
                      imageSize * 0.5f,
                      imageSize * 0.5f);

        svg->draw (g, 1.0f, transform);
    }

    return juce::MouseCursor { image,0,0,6.0f };
}



//======================================================================
//PoZePlot

PoZePlot::PoZePlot()
    : xRange(-1.5f, 1.5f), yRange (-1.5f, 1.5f)
{
    setColour (backgroundColourId, juce::Colours::black);
    setWantsKeyboardFocus (true);
}

PoZePlot::Point* PoZePlot::addPoint (PoZePlot::Point::Type type, float x, float y, bool sendNotification)
{
    auto* point = points.add(std::make_unique<PoZePlot::Point>(type));

    point->setRange (xRange, yRange);
    point->setValue (x, y, true);
    addAndMakeVisible (point);
    resized();

    point->addListener (&pointListener);
    point->addMouseListener (this, true);
    addKeyListener (point);

    if (sendNotification)
        listeners.call([this](Listener& l) { l.pointAdded (this, points.size() - 1); });

    return point;
}

void PoZePlot::removePoint (int index, bool sendNotification)
{
    jassert(isPositiveAndBelow (index, points.size()));

    if (auto* point = getPoint (index); point->isConjugate())
        point->getConjugate()->setConjugate (nullptr);

    removeKeyListener (points.getUnchecked (index));
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

bool PoZePlot::makeConjugatePair (int index, int indexOfConjugate, bool sendNotification)
{
    auto* point = getPoint (index);
    auto* conjugatePoint = getPoint (indexOfConjugate);

    if (point->isConjugate() || conjugatePoint->isConjugate())
        return false;

    point->setConjugate (conjugatePoint);
    conjugatePoint->setConjugate (point);

    if (sendNotification)
        listeners.call([this, index, indexOfConjugate](Listener& l) { l.createdConjugatePair (this, index, indexOfConjugate); });

    return true;
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

    const int pointSize = (int)std::min(bounds.getWidth(), bounds.getHeight()) * 0.04f;

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
    if (event.eventComponent == this)
    // Clicked on PoZePlot
    {
        const float x = event.position.x / (float)getWidth();
        const float y = 1.0f - event.position.y / (float)getHeight();

        if (event.mods.isCommandDown())
            addPoint (PoZePlot::Point::Type::pole, xRange.convertFrom0to1 (x), yRange.convertFrom0to1 (y), true);
        else if (event.mods.isShiftDown())
            addPoint (PoZePlot::Point::Type::zero, xRange.convertFrom0to1 (x), yRange.convertFrom0to1 (y), true);
    }

    if (auto* point = dynamic_cast<PoZePlot::Point*>(event.eventComponent))
    // Clicked on a Pole or Zero
    {
        const int index = points.indexOf (point);

        if (event.mods.isCommandDown() && event.mods.isShiftDown())
        {
            if (! point->isConjugate())
            {
                addPoint (point->getType(), point->getXValue(), -point->getYValue(), true);
                makeConjugatePair (index,  points.size() - 1, true);
            }
        }
        else if (event.mods.isAltDown())
            removePoint (index, true);
    }
}


void PoZePlot::PointListener::pointValueChanged (Point* emitter)
{

}