#pragma once

#include <JuceHeader.h>

class GridAxis : public juce::Component {
public:

    GridAxis() : GridAxis({ 0.0f, 1.0f }, { 0.0f, 1.0f }) {}

    GridAxis(const juce::NormalisableRange<float>& xRange, const juce::NormalisableRange<float>& yRange)
    : xRange(xRange)
    , yRange(yRange)
    {
        setInterceptsMouseClicks(false, false);
    }

    void setRange(const juce::NormalisableRange<float>& range) {
        yRange = range;
        setYTicks(yTicks);
        repaint();
    }

    void setDomain(const juce::NormalisableRange<float>& domain) {
        xRange = domain;
        setXTicks(xTicks);
        repaint();
    }

    void setMargins(float left, float bottom) {
        leftMargin = left;
        bottomMargin = bottom;
        resized();
    }

    void setLineThickness(float thickness) {
        lineThickness = thickness;
        repaint();
    }

    void paint(juce::Graphics& g) override
    {
        g.setColour(backgroundColour);
        g.fillRect(gridArea);

        g.setColour(gridColour);

        for (auto xLine : xLines)
            g.drawLine(xLine, lineThickness);

        for (auto yLine : yLines)
            g.drawLine(yLine, lineThickness);
    }

    void resized() override
    {
        auto bounds = getLocalBounds().toFloat();

        gridArea = bounds;
        gridArea.removeFromLeft(leftMargin);
        gridArea.removeFromBottom(bottomMargin);

        const auto yWidth = juce::jmin<float>(50.0f, bounds.getWidth() * 0.08f);
        const auto xHeight = juce::jmin<float>(50.0f, bounds.getHeight() * 0.0625f);
        xAxis = bounds.removeFromBottom(xHeight);
        yAxis = bounds.removeFromLeft(yWidth);
        setXTicks(xTicks);
        setYTicks(yTicks);
    }

    juce::Rectangle<float> getGridArea() const { return gridArea; }

    void setXTicks(const std::vector<float>& xTicks_)
    {
        xLines.clear();
        xLabels.clear();
        xTicks = xTicks_;
        for (float tick : xTicks) {

            if (tick > xRange.start & tick < xRange.end) {
                const float x = xRange.convertTo0to1(tick) * gridArea.getWidth() + gridArea.getX();
                xLines.emplace_back(x, gridArea.getY(), x, gridArea.getBottom());

                auto label = std::make_unique<juce::Label>("Label", juce::String(tick));
                label->setFont(juce::FontOptions(12.0f));
                label->setSize(50, xAxis.getHeight());
                label->setCentrePosition(x, xAxis.getCentreY());
                label->setJustificationType(juce::Justification::centred);
                label->setColour(juce::Label::ColourIds::textColourId, textColour);
                addAndMakeVisible(*label);
                xLabels.emplace_back(std::move(label));
            }
        }
    }

    void setYTicks(const std::vector<float>& yTicks_)
    {
        yLines.clear();
        yTicks = yTicks_;
        for (float tick : yTicks) {
            if (tick > yRange.start & tick < yRange.end) {
                const float y = (1.0f - yRange.convertTo0to1(tick)) * gridArea.getHeight() + gridArea.getY();
                yLines.emplace_back(gridArea.getX(), y, gridArea.getRight(), y );

                auto label = std::make_unique<juce::Label>("Label", juce::String(tick));
                label->setFont(juce::FontOptions(12.0f));
                label->setSize(yAxis.getWidth(), 20);
                label->setCentrePosition(yAxis.getCentreX(), y);
                label->setJustificationType(juce::Justification::centred);
                label->setColour(juce::Label::ColourIds::textColourId, textColour);
                addAndMakeVisible(*label);
                yLabels.emplace_back(std::move(label));
            }
        }
    }

    void setGridColour(juce::Colour colour)
    {
        gridColour = colour;
        repaint();
    }

    void setTextColour(juce::Colour colour)
    {
        textColour = colour;

        for (auto& l : xLabels)
            l->setColour(juce::Label::ColourIds::textColourId, textColour);

        for (auto& l : yLabels)
            l->setColour(juce::Label::ColourIds::textColourId, textColour);

        repaint();
    }

    void setBackgroundColour(juce::Colour colour)
    {
        backgroundColour = colour;
        repaint();
    }

    const juce::NormalisableRange<float>& getXRange() const { return xRange; };
    const juce::NormalisableRange<float>& getYRange() const { return yRange; };

private:

    juce::NormalisableRange<float> xRange, yRange;
    std::vector<juce::Line<float>> xLines, yLines;
    std::vector<float> xTicks, yTicks;
    std::vector<std::unique_ptr<juce::Label>> xLabels, yLabels;

    juce::Rectangle<float> gridArea;
    juce::Rectangle<float> xAxis, yAxis;

    juce::Colour backgroundColour { juce::Colours::black };
    juce::Colour gridColour { juce::Colours::white.withAlpha(0.5f) };
    juce::Colour textColour { juce::Colours::white };

    float lineThickness { 1.0f };
    float leftMargin { 0.0f };
    float bottomMargin { 0.0f };

};