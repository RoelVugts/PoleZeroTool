#pragma once

#include <JuceHeader.h>

#include "../../DSP/FilterDesign.h"
#include "../../Utils/MappedRange.h"

class Plot : public juce::Component
{
public:

    enum ColourIds
    {
        plotBackgroundColourId      = 0x210100,
        borderOutlineColourId       = 0x210200,
        titleBackgroundColourId    = 0x210300,
        gridColourId                = 0x210400,
        pathColourId                = 0x210500,
        textColourId                = 0x210600
    };

    Plot(const juce::String& plotName) : title(plotName)
    {
        setColour (plotBackgroundColourId,   juce::Colours::black);
        setColour (borderOutlineColourId,    juce::Colours::black);
        setColour (titleBackgroundColourId,  juce::Colour (12, 12, 12));
        setColour (gridColourId,             juce::Colour (60, 60, 60));
        setColour (pathColourId,             juce::Colours::white);
        setColour (textColourId,             juce::Colour (210, 210, 210));
    }

    void paint(juce::Graphics& g) override
    {
        //=======================================================
        // Draw frame and background
        g.setColour (findColour (titleBackgroundColourId));
        g.fillRect (titleArea);

        g.setColour (findColour (borderOutlineColourId));
        g.drawRect (titleArea, borderThickness);

        g.setColour (findColour (plotBackgroundColourId));
        g.fillRect (plotArea);
        g.fillRect (yAxisArea);
        g.fillRect (xAxisArea);

        //=======================================================
        // Draw grid lines
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
            g.drawLine(line, lineThickness);
        }

        // Apply opacity mask over grid lines to fade out to edges
        auto gradColour = findColour (plotBackgroundColourId);
        juce::ColourGradient gradient(gradColour, plotArea.getX(), plotArea.getY(), gradColour, plotArea.getX(), xAxisArea.getY(), false);
        gradient.addColour (0.25f, gradColour.withAlpha (0.0f));
        gradient.addColour (0.75f, gradColour.withAlpha (0.0f));
        g.setGradientFill (gradient);
        g.fillRect (plotArea);

        //=======================================================
        // Draw the path within the path bounds
        {
            juce::Graphics::ScopedSaveState clippedState(g);
            g.reduceClipRegion (plotArea.toNearestInt());
            g.setColour (findColour (pathColourId));
            g.strokePath (path, pathStroke);
        }

        //=======================================================
        // Draw title
        g.setColour (findColour (textColourId));
        g.setFont (juce::FontOptions (13.0f, juce::Font::bold));
        g.drawFittedText (title, titleArea.toNearestInt(), juce::Justification::centred, 1, 0.9f);

        //=======================================================
        // Draw axis labels
        g.setFont (juce::FontOptions (10.0f));
        g.setColour (findColour (textColourId).withAlpha (0.7f));

        for (int i = 0; i < (int)xTicks.size(); i++)
        {
            const juce::String& text = i < (int)xLabels.size() ? xLabels[i] : "";
            int x = (int)(xRange.convertTo0to1 (xTicks[i]) * plotArea.getWidth() + plotArea.getX());
            const int y = (int)xAxisArea.getY();
            const int textWidth = juce::GlyphArrangement::getStringWidthInt (g.getCurrentFont(), text);
            if ((x + textWidth) > getWidth()) x -= ((x + textWidth) - getWidth());
            g.drawFittedText (text, x - textWidth / 2, y, textWidth, (int)xAxisArea.getHeight(), juce::Justification::centred, 1, 0.9f);
        }

        for (int i = 0; i < (int)yTicks.size(); i++)
        {
            const juce::String& text = i < (int)yLabels.size() ? yLabels[i] : "";
            const int textHeight = (int)juce::GlyphArrangement::getStringBounds (g.getCurrentFont(), text).getHeight();
            const int y = (int)((1.0f - yRange.convertTo0to1 (yTicks[i])) * plotArea.getHeight()) + (int)plotArea.getY() - textHeight / 2;
            const int x = (int)yAxisArea.getX() + borderThickness;
            g.drawFittedText (text, x, y, (int)yAxisArea.getWidth() - borderThickness, textHeight, juce::Justification::centred, 1, 0.9f);
        }
    }

    void resized() override
    {
        auto bounds = getLocalBounds().toFloat();
        const float borderSize = std::min(bounds.getWidth() * 0.075f, 25.0f);

        titleArea = bounds.removeFromTop (borderSize);
        yAxisArea = bounds.removeFromLeft (borderSize);
        xAxisArea = bounds.removeFromBottom (borderSize);
                plotArea = bounds;
        bounds.removeFromRight (borderSize);

        juce::NullCheckedInvocation::invoke(dataRefreshFn, getNumDataPoints());

        updatePath();
    }

    void setRange(const MappedRange<float>& range) {
        yRange = range;
        setYTicks(yTicks);
        updatePath();
        repaint();
    }

    void setDomain(const MappedRange<float>& domain) {
        xRange = domain;
        setXTicks(xTicks);
        updatePath();
        repaint();
    }

    void clearData()
    {
        path.clear();
        repaint();
    }

    void updatePath()
    {
        path.clear();

        if (getDataFn == nullptr)
            return;

        const int width = (int)plotArea.getWidth();

        for (int x = 0; x < width; x++)
        {
            const float val = getDataFn(xRange.convertFrom0to1 ((float)x / (float)width));
            float y = 1.0f - yRange.convertTo0to1 (val);
            y = y * plotArea.getHeight() + plotArea.getY();

            if (x == 0)
                path.startNewSubPath (plotArea.getX(), y);
            else
                path.lineTo ((float)x + plotArea.getX(), y);
        }

        repaint();
    }

    void setXTicks(const std::vector<float>& ticks)
    {
        xTicks = ticks;
        repaint();
    }

    void setYTicks(const std::vector<float>& ticks)
    {
        yTicks = ticks;
        repaint();
    }

    void setXLabels(const std::vector<juce::String>& labels)
    {
        xLabels = labels;
        repaint();
    }

    void setYLabels(const std::vector<juce::String>& labels)
    {
        yLabels = labels;
        repaint();
    }

    void setPlotTitle(const juce::String& title_)
    {
        title = title_;
        repaint();
    }

    // Returns the number of data points this plot will query.
    // This will be equal to the amount of (logical) width pixels
    int getNumDataPoints() const { return (int)plotArea.getWidth(); }

    const MappedRange<float>& getXRange() const { return xRange; };
    const MappedRange<float>& getYRange() const { return yRange; };

    /** The plot will call this to get the y value to be drawn.
     *  It will pass in an x value and expects it to return the corresponding
     *  y value.
     *
     *  @param x                The denormalized x value
     *  @returns                The denormalized y value
     */
    std::function<float(float x)> getDataFn { nullptr };

    std::function<void(int numDataPoints)> dataRefreshFn { nullptr };

private:
    juce::String title;

    juce::Path path;
    juce::PathStrokeType pathStroke { 1.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded };

    MappedRange<float> xRange, yRange;
    std::vector<float> xTicks, yTicks;
    std::vector<juce::String> xLabels, yLabels;

    juce::Rectangle<float> plotArea;
    juce::Rectangle<float> xAxisArea, yAxisArea, titleArea;

    float lineThickness { 0.5f };
    static constexpr int borderThickness { 2 };
};