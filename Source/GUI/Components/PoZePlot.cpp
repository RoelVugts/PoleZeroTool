#include "PoZePlot.h"

#include "../LookAndFeel.h"

const int PoZePlot::Point::angleKeyCode { juce::KeyPress::createFromDescription ("r").getKeyCode() };
const int PoZePlot::Point::magKeyCode { juce::KeyPress::createFromDescription ("m").getKeyCode() };

PoZePlot::Point::Point (Type type_) : type(type_)
{
    SettableTooltipClient::setTooltip (getType() == PoZePlot::Point::Type::pole ? "Pole" : "Zero");
    setWantsKeyboardFocus (true);
}

//======================================================================

void PoZePlot::Point::setValue (double x_, double y_, bool sendNotification)
{
    // Out of range !
    jassert(x_ >= xRange.start && x_ <= xRange.end);
    jassert(y_ >= yRange.start && y_ <= yRange.end);

    // Prevent infinite loop when updating conjugate
    if (approximatelyEqual (x, x_) && approximatelyEqual (y, y_))
        return;

    x = x_;
    y = y_;
    updatePosition();

    if (sendNotification)
        listeners.call([this](Listener& l) { l.pointValueChanged (this); });

    if (conjugate != nullptr)
        conjugate->setValue (x, -y, true);
}

void PoZePlot::Point::setXValue (double value, bool sendNotification)
{
    setValue (value, y, sendNotification);
}

void PoZePlot::Point::setYValue (double value, bool sendNotification)
{
    setValue (x, value, sendNotification);
}

void PoZePlot::Point::setNormalizedValue(double x_, double y_, bool sendNotification)
{
    // Expects normalized values !
    jassert(x_ >= 0.0 && x_ <= 1.0);
    jassert(y_ >= 0.0 && y_ <= 1.0);

    setValue (xRange.convertFrom0to1 (x_), yRange.convertFrom0to1 (y_), sendNotification);
}

void PoZePlot::Point::setRange (const juce::NormalisableRange<double>& xRange_, const juce::NormalisableRange<double>& yRange_)
{
    xRange = xRange_;
    yRange = yRange_;
    updatePosition();
}

juce::Point<double> PoZePlot::Point::getValue() const noexcept
{
    return { getXValue(), getYValue() };
}

double PoZePlot::Point::getXValue() const noexcept
{
    return x;
}

double PoZePlot::Point::getYValue() const noexcept
{
    return y;
}

double PoZePlot::Point::getAngle() const noexcept
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
        const int newY = (int)std::round(parentBounds.getHeight() * (1.0 - yRange.convertTo0to1 (y)));
        setCentrePosition(newX, newY);
    }
}

//======================================================================

void PoZePlot::Point::mouseDown(const juce::MouseEvent& event)
{
    if (dragMode == DragMode::normal)
    {
        dragger.startDraggingComponent(this, event);
        setMouseCursor (juce::MouseCursor::NoCursor);
    }

    valueBeforeDrag = getValue();
}

void PoZePlot::Point::mouseDrag(const juce::MouseEvent& event)
{
    if (auto* parent = getParentComponent())
    {
        const auto parentBounds = parent->getLocalBounds().toDouble();

        switch (dragMode)
        {
            case DragMode::normal:
            {
                const bool constrainToRealAxis = event.mods.isCommandDown();
                if (! constrainToRealAxis)
                    dragger.dragComponent(this, event, &constrainer);

                const double xNorm = (constrainToRealAxis ? (double)parent->getMouseXYRelative().getX()
                                                         : (double)getBounds().getCentreX()) / parentBounds.getWidth();
                double yNorm = constrainToRealAxis ? 0.5 : 1.0f - ((double)getBounds().getCentreY() / parentBounds.getHeight());

                setNormalizedValue (xNorm, yNorm, true);
                break;
            }

            case DragMode::angle:
            {
                // Get unit circle center (origin)
                const auto origin = parentBounds.getCentre();

                // Get angle from origin
                const auto point = parent->getMouseXYRelative().toDouble() - origin;

                const double newAngle = -std::atan2(point.y, point.x);
                const double mag = valueBeforeDrag.getDistanceFromOrigin();
                const double xValue = std::clamp(mag * std::cos(newAngle), xRange.start, xRange.end);
                const double yValue = std::clamp(mag * std::sin(newAngle), yRange.start, yRange.end);
                setValue (xValue, yValue, true);
                break;
            }

            case DragMode::magnitude:
            {
                const double angleOnMouseDown = std::atan2(valueBeforeDrag.y, valueBeforeDrag.x);
                const juce::Point<double> dir (std::cos(angleOnMouseDown), std::sin(angleOnMouseDown));

                // Mouse position in value space
                const auto point = parent->getMouseXYRelative().toDouble();
                const double xNorm = std::clamp(point.x / parentBounds.getWidth(), 0.0, 1.0);
                const double yNorm = std::clamp(1.0 - (point.y / parentBounds.getHeight()), 0.0, 1.0);
                const juce::Point<double> mouseValue (xRange.convertFrom0to1(xNorm),yRange.convertFrom0to1(yNorm));

                double magnitude = mouseValue.x * dir.x + mouseValue.y * dir.y;

                // Constrain magnitude
                const double xValue = std::clamp(magnitude * dir.x, xRange.start, xRange.end);
                const double yValue = std::clamp(magnitude * dir.y, yRange.start, yRange.end);
                const double newAngle = std::atan2(yValue, xValue);

                if (approximatelyEqual (angleOnMouseDown, newAngle))
                    setValue(xValue,yValue, true);

                break;
            }
        }
    }
}

void PoZePlot::Point::mouseUp(const juce::MouseEvent& event)
{
    juce::ignoreUnused (event);
}

void PoZePlot::Point::mouseEnter (const juce::MouseEvent&)
{
    updateDragMode();
    grabKeyboardFocus();

    mouseIsOver = true;
    repaint();
}

void PoZePlot::Point::mouseExit (const juce::MouseEvent&)
{
    mouseIsOver = false;
    setMouseCursor (juce::MouseCursor::NormalCursor);
    repaint();
}

bool PoZePlot::Point::keyPressed (const juce::KeyPress& key, juce::Component*)
{
    if (key.getKeyCode() == angleKeyCode && dragMode != DragMode::angle)
    {
        dragMode = DragMode::angle;

        if (auto* lf = getCustomLookAndFeel())
            setMouseCursor (rotateCursor);

        valueBeforeDrag = getValue();
    }
    else if (key.getKeyCode() == magKeyCode && dragMode != DragMode::magnitude)
    {
        dragMode = DragMode::magnitude;

        setMouseCursor (getRotatedMagnitudeCursor ((float)getAngle()));

        valueBeforeDrag = getValue();
    }

    return true;
}

bool PoZePlot::Point::keyStateChanged (bool isKeyDown, Component*)
{
    if (! isKeyDown)
        updateDragMode();

    return true;
}

void PoZePlot::Point::updateDragMode()
{
    if (juce::KeyPress::isKeyCurrentlyDown (angleKeyCode))
    {
        if (auto* lf = getCustomLookAndFeel())
            setMouseCursor (rotateCursor);

        dragMode = DragMode::angle;
    }
    else if (juce::KeyPress::isKeyCurrentlyDown (magKeyCode))
    {
        setMouseCursor (getRotatedMagnitudeCursor ((float)getAngle()));

        dragMode = DragMode::magnitude;
    }
    else
    {
        setMouseCursor (juce::MouseCursor::NormalCursor);
        dragMode = DragMode::normal;
    }
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
    : xRange(-1.5, 1.5), yRange (-1.5, 1.5)
{
    setColour (backgroundColourId, juce::Colours::black);
    setColour (unitCircleColourId, juce::Colours::white);
    setWantsKeyboardFocus (true);
}

PoZePlot::Point* PoZePlot::addPoint (PoZePlot::Point::Type type, double x, double y, bool sendNotification)
{
    auto* point = points.add(std::make_unique<PoZePlot::Point>(type));

    point->setRange (xRange, yRange);
    point->setValue (x, y, true);
    addAndMakeVisible (point);
    resized();

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


void PoZePlot::setRange (const juce::NormalisableRange<double>& xRange_, const juce::NormalisableRange<double>& yRange_)
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
    static constexpr float lineThickness = 0.5f;

    // Draw unit circle
    g.setColour(findColour (unitCircleColourId));
    g.drawEllipse(unitCircleArea, lineThickness);

    // Draw axis lines
    g.drawLine(bounds.getWidth() * 0.05f,bounds.getHeight() * 0.5f, bounds.getWidth() * 0.95f, bounds.getHeight() * 0.5f, lineThickness);
    g.drawLine(bounds.getWidth() / 2.0f, bounds.getHeight() * 0.05f, bounds.getWidth() / 2.0f, bounds.getHeight() * 0.95f, lineThickness);
}

void PoZePlot::resized()
{
    const auto bounds = getLocalBounds().toFloat();
    const float cornerSize = std::min(bounds.getWidth(), bounds.getHeight()) * 0.08f;
    setRoundedCorners (cornerSize);

    const int pointSize = (int)(std::min(bounds.getWidth(), bounds.getHeight()) * 0.04f);

    for (auto* point : points)
    {
        point->setSize (pointSize, pointSize);
        point->updatePosition();
    }

    unitCircleArea.setX ((float)xRange.convertTo0to1 (-1.0) * bounds.getWidth());
    unitCircleArea.setRight ((float)xRange.convertTo0to1 (1.0) * bounds.getWidth());
    unitCircleArea.setTop ((float)yRange.convertTo0to1 (-1.0) * bounds.getHeight());
    unitCircleArea.setBottom ((float)yRange.convertTo0to1 (1.0) * bounds.getHeight());
}

//======================================================================
void PoZePlot::mouseDown (const juce::MouseEvent& event)
{
    const double x = event.position.x / (double)getWidth();
    const double y = 1.0 - event.position.y / (double)getHeight();

    if (event.mods.isRightButtonDown())
    {
        juce::PopupMenu menu;
        if (auto* point = dynamic_cast<PoZePlot::Point*>(event.eventComponent))
        {
            const int index = points.indexOf (point);

            if (! point->isConjugate())
                menu.addItem("Add Conjugate", [this, point, index]() {
                    addPoint (point->getType(), point->getXValue(), -point->getYValue(), true);
                    makeConjugatePair (index,  points.size() - 1, true);
                });

            menu.addItem("Delete", [this, index]() {
                removePoint (index, true);
            });
        }
        else
        {
            menu.addItem("Add Pole", [this, x, y]() {
                addPoint (PoZePlot::Point::Type::pole, xRange.convertFrom0to1 (x), yRange.convertFrom0to1 (y), true);
            });

            menu.addItem("Add Zero", [this, x, y]() {
                addPoint (PoZePlot::Point::Type::zero, xRange.convertFrom0to1 (x), yRange.convertFrom0to1 (y), true);
            });
        }
        menu.showMenuAsync (juce::PopupMenu::Options().withMousePosition());
    }
    else
    {
        if (event.eventComponent == this)
            // Clicked on PoZePlot
        {
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

}