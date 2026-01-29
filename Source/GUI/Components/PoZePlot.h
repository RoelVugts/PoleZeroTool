#pragma once

#include "RoundedCornerComponent.h"

#include <JuceHeader.h>

class PoZeToolLaf;

class PoZePlot : public RoundedCornerComponent, public juce::SettableTooltipClient
{
public:

    //======================================================================
    // Point in a Pole-Zero plot, representing either a Pole or a Zero
    class Point : public RoundedCornerComponent, public juce::KeyListener, public juce::SettableTooltipClient
    {
    public:

        enum class Type { zero, pole };
        enum class DragMode { normal, angle, magnitude };

        class Listener
        {
        public:
            Listener() = default;
            virtual ~Listener() = default;
            virtual void pointValueChanged ([[maybe_unused]] Point* emitter) {}
            JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Listener)
        };

        void addListener (Listener* listener) { listeners.add (listener); }
        void removeListener (Listener* listener) { listeners.remove (listener); }

        //======================================================================
        explicit Point(Type type);

        //======================================================================

        /** Set point value.
         *
         * @param x                         x value, should be in the current x range
         * @param y                         y value, should be in the current y range
         * @param sendNotification          Send notification to listeners
         */
        void setValue(double x, double y, bool sendNotification);

        /** Set point X value.
         *
         * @param value                      x value, should be in the current x range
         * @param sendNotification          Send notification to listeners
         */
        void setXValue(double value, bool sendNotification);

        /** Set point Y value.
         *
         * @param value                     y value, should be in the current y range
         * @param sendNotification          Send notification to listeners
         */
        void setYValue(double value, bool sendNotification);

        /** Set point with normalized values.
         *
         * @param x                         Ranging from 0 (left) to 1 (right)
         * @param y                         Ranging from 0 (bottom) to 1 (top)
         * @param sendNotification          Send notification to listeners
         */
        void setNormalizedValue (double x, double y, bool sendNotification);

        /** Sets the x and y range of the points value.
         *
         * @param xRange                    The x (horizontal / real) range
         * @param yRange                    The y (vertical / imaginary) range
         */
        void setRange(const juce::NormalisableRange<double>& xRange, const juce::NormalisableRange<double>& yRange);

        // Set this to be painted as a pole or a zero.
        void setType(Type type);

        //======================================================================
        /** Returns the normalized position of the point.
         *
         *  @returns                        Normalized position where 0.0 is left or bottom,
         *                                  1.0 is right or top
         */
        juce::Point<double> getValue() const noexcept;

        // Returns the points Y value on the plot.
        double getXValue() const noexcept;

        // Returns the points X value on the plot.
        double getYValue() const noexcept;

        double getAngle() const noexcept;

        // Returns the type (Pole or Zero)
        Type getType() const { return type; }

        void setConjugate(Point* conjugatePont);

        bool isConjugate() const { return conjugate != nullptr; }
        Point* getConjugate() const { return conjugate; }

        void updatePosition();
        //======================================================================
        void paintWithinCorners (juce::Graphics& g) override;
        void resized() override;

        //======================================================================
        class LookAndFeelMethods
        {
        public:
            virtual ~LookAndFeelMethods() = default;
            virtual void drawPoZePoint (juce::Graphics& g, Point& p, bool mouseIsOverOrDragging) = 0;
        };

    private:
        //======================================================================
        void mouseDown (const juce::MouseEvent& event) override;
        void mouseDrag (const juce::MouseEvent& event) override;
        void mouseUp (const juce::MouseEvent& event) override;
        void mouseEnter(const MouseEvent& event) override;
        void mouseExit(const MouseEvent& event) override;

        //======================================================================
        bool keyPressed(const KeyPress& key, Component* originatingComponent) override;
        bool keyStateChanged(bool isKeyDown, Component* originatingComponent) override;
        void updateDragMode();
        //======================================================================
        PoZeToolLaf* getCustomLookAndFeel() const;
        static juce::MouseCursor getRotatedMagnitudeCursor(float angle);

        juce::NormalisableRange<double> xRange;
        juce::NormalisableRange<double> yRange;

        double x { 0.0f };
        double y { 0.5f };

        juce::ComponentDragger dragger;
        juce::ComponentBoundsConstrainer constrainer;
        juce::ListenerList<Listener> listeners;
        Type type { Type::pole };
        Point* conjugate { nullptr };

        bool mouseIsOver { false };
        DragMode dragMode { DragMode::normal };
        juce::Point<double> valueBeforeDrag {};

        static const int angleKeyCode;
        static const int magKeyCode;

        const juce::MouseCursor rotateCursor { juce::ImageFileFormat::loadFrom(BinaryData::rotateCursor_png, BinaryData::rotateCursor_pngSize), 0, 0, 6.0f };

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Point)
    };

    //======================================================================
    //PoZePlot

    //======================================================================
    class Listener
    {
    public:
        Listener() = default;
        virtual ~Listener() = default;
        virtual void pointAdded ([[maybe_unused]] PoZePlot* emitter, [[maybe_unused]] int indexOfAddedPoint) {}
        virtual void pointRemoved ([[maybe_unused]] PoZePlot* emitter, [[maybe_unused]] int indexOfRemovedPoint) {}
        virtual void createdConjugatePair ([[maybe_unused]] PoZePlot* emitter, [[maybe_unused]] int index, [[maybe_unused]] int conjugateIndex) {}
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Listener)
    };

    void addListener (Listener* listener) { listeners.add (listener); }
    void removeListener (Listener* listener) { listeners.remove (listener); }

    //======================================================================
    enum ColourIds
    {
        backgroundColourId      = 0x200100,
        unitCircleColourId      = 0x200101
    };

    //======================================================================
    PoZePlot();

    //======================================================================
    /** Adds a Pole or Zero to the plot.
     *
     * @param type                          If the added point should be a Pole or Zero
     * @param x                             The x (real) value
     * @param y                             The y (imag) value
     * @param sendNotification              Send notification to listeners
     */
    Point* addPoint (Point::Type type, double x, double y, bool sendNotification);

    /** Removes a Pole or Zero from the plot.
     *
     * @param index                         The index to remove.
     */
    void removePoint(int index, bool sendNotification);

    /** Removes all the Poles and Zeros from the plot.
     *
     * @param sendNotification              Send notification to listeners
     */
    void removeAllPoints(bool sendNotification);

    bool makeConjugatePair(int index, int conjugateIndex, bool sendNotification);

    //======================================================================
    /** Sets the x and y range of the Pole-Zero plot.
     *
     * @param xRange                    The x (horizontal / real) range
     * @param yRange                    The y (vertical / imaginary) range
     */
    void setRange(const juce::NormalisableRange<double>& xRange, const juce::NormalisableRange<double>& yRange);

    //======================================================================
    void paintWithinCorners (juce::Graphics& g) override;
    void resized() override;

    //======================================================================
    Point* getPoint(int index);
    int getNumPoints() const { return points.size(); }
    juce::OwnedArray<Point>& getPoints() { return points; };

    juce::NormalisableRange<double> getXRange() const { return xRange; }
    juce::NormalisableRange<double> getYRange() const { return yRange; }

private:

    void mouseDown(const juce::MouseEvent& event) override;

    juce::OwnedArray<PoZePlot::Point> points;
    juce::NormalisableRange<double> xRange {}, yRange {};
    juce::Rectangle<float> unitCircleArea;

    juce::ListenerList<Listener> listeners;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PoZePlot)
};