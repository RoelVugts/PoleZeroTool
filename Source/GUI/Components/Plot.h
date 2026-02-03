#pragma once

#include <JuceHeader.h>

#include "DragBox.h"
#include "../../Utils/MappedRange.h"

/** Visualizes data on a 2-dimensional plot.*/
class Plot : public RoundedCornerComponent, private DragBox::Listener, public juce::SettableTooltipClient
{
public:

    class Listener
    {
    public:
        Listener() = default;
        virtual ~Listener() = default;
        virtual void rangeChanged(Plot*) {}
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Listener)
    };

    void addListener (Listener* listener) { listeners.add (listener); }
    void removeListener (Listener* listener) { listeners.remove (listener); }

    enum ColourIds
    {
        plotBackgroundColourId      = 0x210100,
        borderOutlineColourId       = 0x210200,
        titleBackgroundColourId    = 0x210300,
        gridColourId                = 0x210400,
        pathColourId                = 0x210500,
        textColourId                = 0x210600
    };

    Plot(const juce::String& plotName, float minRange, float maxRange)
        : title(plotName)
    {
        setColour (plotBackgroundColourId,   juce::Colours::black);
        setColour (borderOutlineColourId,    juce::Colours::black);
        setColour (titleBackgroundColourId,  juce::Colour (12, 12, 12));
        setColour (gridColourId,             juce::Colour (60, 60, 60));
        setColour (pathColourId,             juce::Colours::white);
        setColour (textColourId,             juce::Colour (210, 210, 210));

        setMinMaxRange (minRange, maxRange);
        setRange({ minRange, maxRange }, false);

        minRangeBox.addListener (this);
        maxRangeBox.addListener (this);

        setRoundedCorners (10.0f);
        setStrokeThickness (2.0f);
    }

    void paintWithinCorners(juce::Graphics& g) override
    {
        //=======================================================
        // Draw background
        g.setColour (findColour (plotBackgroundColourId));
        g.fillRect (plotArea);
        g.fillRect (yAxisArea);
        g.fillRect (xAxisArea);
        g.fillRect (headerArea);

        //=======================================================
        // Draw grid lines
        g.setColour (findColour (gridColourId));

        g.setColour (findColour (gridColourId));

        for (const float xTick : xTicks)
        {
            const float x = xRange.convertTo0to1 (xTick) * plotArea.getWidth() + plotArea.getX();
            const juce::Line<float> line(x, plotArea.getY(), x, xAxisArea.getY());
            g.drawLine(line, lineThickness);
        }

        for (const float yTick : yTicks)
        {
            const float y = (1.0f - yRange.convertTo0to1 (yTick)) * plotArea.getHeight() + plotArea.getY();
            const juce::Line<float> line(yAxisArea.getRight(), y, plotArea.getRight(), y);
            if (y > 0.0f && y < (float)getHeight())
                g.drawLine(line, lineThickness);
        }

        // Draw axis labels
        g.setFont (juce::FontOptions (10.0f));
        g.setColour (findColour (textColourId).withAlpha (0.7f));

        for (int i = 0; i < (int)xTicks.size(); i++)
        {
            const juce::String& text = i < (int)xLabels.size() ? xLabels[(size_t)i] : "";
            int x = (int)(xRange.convertTo0to1 (xTicks[(size_t)i]) * plotArea.getWidth() + plotArea.getX());
            const int y = (int)xAxisArea.getY();
            const int textWidth = juce::GlyphArrangement::getStringWidthInt (g.getCurrentFont(), text);
            if ((x + textWidth) > getWidth()) x -= ((x + textWidth) - getWidth());
                g.drawFittedText (text, x - textWidth / 2, y, textWidth, (int)xAxisArea.getHeight(), juce::Justification::centred, 1, 0.9f);
        }

        for (int i = 0; i < (int)yTicks.size(); i++)
        {
            const juce::String& text = i < (int)yLabels.size() ? yLabels[(size_t)i] : "";
            const int textHeight = (int)juce::GlyphArrangement::getStringBounds (g.getCurrentFont(), text).getHeight();
            const int y = (int)((1.0f - yRange.convertTo0to1 (yTicks[(size_t)i])) * plotArea.getHeight()) + (int)plotArea.getY() - textHeight / 2;
            const int x = (int)yAxisArea.getX() + (int)getStrokeThickness();
            if (y > 0 && y < getHeight())
                g.drawFittedText (text, x, y, (int)yAxisArea.getWidth() - (int)getStrokeThickness(), textHeight, juce::Justification::centred, 1, 0.6f);
        }

        //=======================================================
        // Draw the path within the path bounds
        {
            juce::Graphics::ScopedSaveState clippedState(g);
            g.reduceClipRegion (plotArea.toNearestInt());
            g.setColour (findColour (pathColourId));
            g.strokePath (path, pathStroke);
        }

        //=======================================================
        // Apply opacity mask to fade edges
        auto gradColour = findColour (plotBackgroundColourId);
        juce::ColourGradient gradient(gradColour, plotArea.getX(), plotArea.getY(), gradColour, plotArea.getX(), xAxisArea.getY(), false);
        gradient.addColour (0.15f, gradColour.withAlpha (0.0f));
        gradient.addColour (0.85f, gradColour.withAlpha (0.0f));
        g.setGradientFill (gradient);
        g.fillRect (getLocalBounds());

        //=======================================================
        // Draw x axis labels
        g.setFont (juce::FontOptions (10.0f));
        g.setColour (findColour (textColourId).withAlpha (0.7f));

        for (int i = 0; i < (int)xTicks.size(); i++)
        {
            const juce::String& text = i < (int)xLabels.size() ? xLabels[(size_t)i] : "";
            int x = (int)(xRange.convertTo0to1 (xTicks[(size_t)i]) * plotArea.getWidth() + plotArea.getX());
            const int y = (int)xAxisArea.getY();
            const int textWidth = juce::GlyphArrangement::getStringWidthInt (g.getCurrentFont(), text);
            if ((x + textWidth) > getWidth()) x -= ((x + textWidth) - getWidth());
            g.drawFittedText (text, x - textWidth / 2, y, textWidth, (int)xAxisArea.getHeight(), juce::Justification::centred, 1, 0.9f);
        }

        //=======================================================
        // Draw frame and title area
        g.setColour (findColour (titleBackgroundColourId));
        g.fillRect (headerArea);

        g.setColour (findColour (borderOutlineColourId));
        // g.drawRect (headerArea, borderThickness);
        // g.drawRect (getLocalBounds(), borderThickness);

        //=======================================================
        // Draw title
        g.setColour (findColour (textColourId));
        g.setFont (juce::FontOptions (13.0f, juce::Font::bold));
        g.drawFittedText (title, titleArea.toNearestInt(), juce::Justification::centred, 1, 0.9f);

        g.setColour (findColour (textColourId).darker ());
        g.setFont (juce::FontOptions (11.0f));
        g.drawFittedText (unit, unitArea.toNearestInt(), juce::Justification::centredLeft, 1, 0.9f);

        //=======================================================

        addAndMakeVisible (maxRangeBox);
        addAndMakeVisible (minRangeBox);
    }

    void resized() override
    {
        auto bounds = getLocalBounds().toFloat();
        const float borderSize = std::min(bounds.getWidth() * 0.075f, 25.0f);

        headerArea = bounds.removeFromTop (borderSize);
        titleArea = headerArea.withSizeKeepingCentre (bounds.getWidth() * 0.5f, headerArea.getHeight());
        yAxisArea = bounds.removeFromLeft (borderSize + 10.0f);
        xAxisArea = bounds.removeFromBottom (borderSize);
        plotArea = bounds;
        bounds.removeFromRight (borderSize);

        auto rangeArea = headerArea;
        const float titleRight = titleArea.getCentreX() + juce::GlyphArrangement::getStringWidth (juce::FontOptions(10.0f), title) * 0.5f;
        rangeArea.removeFromLeft (titleRight);
        rangeArea.reduce(10.0f, titleArea.getHeight() * 0.2f);

        auto unitTextArea = headerArea;
        unitArea = unitTextArea.removeFromLeft (titleArea.getX());
        unitArea.removeFromLeft (10.0f);

        const float rangeBoxWidth = rangeArea.getWidth() * 0.3f;
        const float spacing = rangeArea.getWidth() * 0.05f;

        auto maxRangeArea = rangeArea.removeFromRight (rangeBoxWidth);
        maxRangeBox.setBounds (maxRangeArea.toNearestInt());
        rangeArea.removeFromRight (spacing);
        auto minRangeArea = rangeArea.removeFromRight (rangeBoxWidth);
        minRangeBox.setBounds (minRangeArea.toNearestInt());

        juce::NullCheckedInvocation::invoke(onNumDataPointsChanged, getNumDataPoints());

        updatePath();
    }

    /** Sets the range (y-axis) of the plot.
     *  @param range                The new range for the y axis
     *  @param sendNotification     If true, will trigger a callback to the listeners
     */
    void setRange(const MappedRange<float>& range, bool sendNotification) {
        yRange = range;
        maxRangeBox.setValue (yRange.end, true);
        minRangeBox.setValue (yRange.start, true);
        setYTicks(yTicks);
        updatePath();
        repaint();

        if (sendNotification)
            listeners.call([this](Listener& l) { l.rangeChanged (this); });
    }

    /** Set the domain (x axis) of the plot.
     *  @param domain                The new range for the x axis
     */
    void setDomain(const MappedRange<float>& domain) {
        xRange = domain;
        setXTicks(xTicks);
        updatePath();
        repaint();
    }

    /** Set the maximum allowed range for the plot.
     *  This might be confusing since the setRange and setDomain functions also exist,
     *  but those set the current visible range.\n\n
     *
     *  This sets the maximum allowed range for the plot, the user can use both drag boxes
     *  to set the current visible range within the maximum allowed range.
     *
     *  @param minValue                Lower limit of the range
     *  @param maxValue                Higher limit of the range
     */
    void setMinMaxRange(float minValue, float maxValue)
    {
        minRangeBox.setRange ({ minValue, maxValue - minSpan });
        maxRangeBox.setRange({ minValue + minSpan, maxValue });
    }

    /** Will force the path to query for new data and repaint itself.*/
    void updatePath()
    {
        path.clear();
        const int width = (int)plotArea.getWidth();

        if (getDataFn == nullptr || width == 0)
            return;


        for (int x = 0; x < width; x++)
        {
            const float xValue = xRange.convertFrom0to1 ((float)x / (float)(width - 1));
            const float val = getDataFn(xValue, x);
            float y = 1.0f - yRange.convertTo0to1 (val);
            y = y * plotArea.getHeight() + plotArea.getY();

            if (x == 0)
                path.startNewSubPath (plotArea.getX(), y);
            else
                path.lineTo ((float)x + plotArea.getX(), y);
        }

        repaint();
    }

    /** Sets the vertical grid lines.
     *  @param ticks            The values at which the lines should be displayed.
     */
    void setXTicks(const std::vector<float>& ticks)
    {
        xTicks = ticks;
        repaint();
    }

    /** Sets the horizontal grid lines.
     *  @param ticks            The values at which the lines should be displayed.
     */
    void setYTicks(const std::vector<float>& ticks)
    {
        yTicks = ticks;
        repaint();
    }

    /** Sets the lables to display next to the vertical grid lines.
     *  @param labels            The strings to display. Note that the string will be displayed
     *                           at the index of the corresponding grid line.
     * @see setXTicks
     */
    void setXLabels(const std::vector<juce::String>& labels)
    {
        xLabels = labels;
        repaint();
    }

    /** Sets the lables to display next to the horizontal grid lines.
     *  @param labels            The strings to display. Note that the string will be displayed
     *                           at the index of the corresponding grid line.
     * @see setYTicks
     */
    void setYLabels(const std::vector<juce::String>& labels)
    {
        yLabels = labels;
        repaint();
    }

    // Sets the title to display at the top of the plot
    void setPlotTitle(const juce::String& title_)
    {
        title = title_;
        repaint();
    }

    // Sets the small text at the top left that indicates which unit the range
    // is currently displaying.
    void setUnitText(const juce::String& text)
    {
        unit = text;
        repaint();
    }

    /** Returns the number of data points this plot will query.
     *  This will be equal to the amount of (logical) width pixels
     */
    int getNumDataPoints() const { return (int)plotArea.getWidth(); }

    // Returns the current visible range
    const MappedRange<float>& getXRange() const { return xRange; }

    // Returns the current visible range
    const MappedRange<float>& getYRange() const { return yRange; }

    /** The plot will call this to get the y value to be drawn.
     *  It will pass in an x value and expects it to return the corresponding
     *  y value.
     *
     *  @param x                The denormalized x value
     *  @param index            Index of the data point
     *  @returns                The denormalized y value
     */
    std::function<float(float x, int index)> getDataFn { nullptr };

    // Called when the number of data points the plot needs is changed.
    std::function<void(int numDataPoints)> onNumDataPointsChanged { nullptr };

    DragBox& getMinRangeBox() { return minRangeBox; }
    DragBox& getMaxRangeBox() { return maxRangeBox; }

private:

    void valueChanged(DragBox* box) override
    {
        if (box == &minRangeBox)
        {
            if (minRangeBox.getValue() >= (maxRangeBox.getValue() - minSpan))
                minRangeBox.setValue (maxRangeBox.getValue() - minSpan, true);

            setRange (MappedRange<float>(box->getValue(), yRange.end), true);
        }
        else if (box == &maxRangeBox)
        {
            if (maxRangeBox.getValue() <= (minRangeBox.getValue() + minSpan))
                maxRangeBox.setValue (minRangeBox.getValue() + minSpan, true);

            setRange (MappedRange<float>(yRange.start, box->getValue()), true);
        }
    }

    juce::String title;
    juce::String unit;

    juce::Path path;
    juce::PathStrokeType pathStroke { 1.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded };

    MappedRange<float> xRange, yRange;
    std::vector<float> xTicks, yTicks;
    std::vector<juce::String> xLabels, yLabels;

    juce::Rectangle<float> plotArea, headerArea;
    juce::Rectangle<float> xAxisArea, yAxisArea, titleArea, unitArea;

    DragBox maxRangeBox;
    DragBox minRangeBox;

    juce::ListenerList<Listener> listeners;

    float lineThickness { 0.5f };
    static constexpr float minSpan { 1e-3f };
};