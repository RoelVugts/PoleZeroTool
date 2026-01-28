#pragma once

#include <JuceHeader.h>

#include "../../Data/Attachments/PlotRangeAttachment.h"
#include "../../Data/Attachments/PlotDataAttachment.h"
#include "../Components/Plot.h"
#include "../DSP/MathFunctions.h"
#include "../LookAndFeel.h"

#include <magic_enum/magic_enum.hpp>

class ResponsePlotSection : public juce::Component, private Plot::Listener
{
public:

    ResponsePlotSection(AudioPluginAudioProcessor& p)
        : processor(p), state(p.state)
    {

        for (auto* plot : juce::Array<Plot*>{&firstPlot, &secondPlot})
        {
            plot->addListener (this);
            addAndMakeVisible (plot);
        }

        //==================================================================================================
        plotAttachment = std::make_unique<PlotDataAttachment> (p.state, p.filterDesign, firstPlot, secondPlot);
        firstPlotRangeAttachment  = std::make_unique<PlotRangeAttachment>(state, firstPlot,  0);
        secondPlotRangeAttachment = std::make_unique<PlotRangeAttachment>(state, secondPlot, 1);

        //==================================================================================================
        state.setOnPropertyChanged (State::IDs::firstPlotType, [this]() {
            const auto selectedType = state.firstPlotType.getValue();
            const juce::String& title = magic_enum::enum_name(selectedType).data();
            firstPlot.setPlotTitle (title);
            firstPlotRangeAttachment->updateRange();
            firstPlot.updatePath();
        }, true);

        state.setOnPropertyChanged (State::IDs::secondPlotType, [this]() {
            const auto selectedType = state.secondPlotType.getValue();
            const juce::String& title = magic_enum::enum_name(selectedType).data();
            secondPlot.setPlotTitle (title);
            secondPlotRangeAttachment->updateRange();
            secondPlot.updatePath();
        }, true);

        //==================================================================================================
        state.setOnPropertyChanged (State::IDs::displayLogarithmic, [this]() {
            const bool isLogarithmic = state.displayLogarithmic.getValue();
            plotAttachment->updateResponse();
            setLogDomain (isLogarithmic);
        }, true);

        state.setOnPropertyChanged (State::IDs::displayInHz, [this]() {
            setDisplayedUnit (state.displayInHz.getValue());
        }, true);

        processor.onSampleRateChange = [this](double) { setDisplayedUnit (state.displayInHz.getValue()); };
    }

    void resized() override
    {
        auto bounds = getLocalBounds().toFloat();
        const float height = bounds.getHeight();

        const float plotHeight = (height - LAF::Layout::defaultSpacing) * 0.5f;
        auto magPlotArea = bounds.removeFromTop (plotHeight);
        firstPlot.setBounds (magPlotArea.toNearestInt());

        bounds.removeFromTop (LAF::Layout::defaultSpacing);
        auto phasePlotArea = bounds;
        secondPlot.setBounds (phasePlotArea.toNearestInt());
    }

    Plot firstPlot { "Magnitude" , -100.0f, 100.0f };
    Plot secondPlot { "Phase" , -1000.0f, 1000.0f };


private:

    void setLogDomain(bool shouldDisplayLogarithmic)
    {
        if (shouldDisplayLogarithmic)
        {
            MappedRange<float> logRange = MappedRange<float>::createExponentialRange (0.0f, juce::MathConstants<float>::pi);

            for (auto* plot : juce::Array<Plot*>{&firstPlot, &secondPlot})
            {
                plot->setDomain (logRange);
                plot->setXTicks ({
                    juce::MathConstants<float>::halfPi   * 0.00390625f,
                    juce::MathConstants<float>::halfPi   * 0.0078125f,
                    juce::MathConstants<float>::halfPi   * 0.015625f,
                    juce::MathConstants<float>::halfPi   * 0.03125f,
                    juce::MathConstants<float>::halfPi   * 0.0625f,
                    juce::MathConstants<float>::halfPi   * 0.125f,
                    juce::MathConstants<float>::halfPi   * 0.25f,
                    juce::MathConstants<float>::halfPi   * 0.5f,
                    juce::MathConstants<float>::halfPi,
                    juce::MathConstants<float>::pi
                });
            }
        }
        else
        {
            for (auto* plot : juce::Array<Plot*>{&firstPlot, &secondPlot})
            {
                plot->setDomain ({ 0.0f, juce::MathConstants<float>::pi });
                plot->setXTicks ({
                    MathFunctions::roundToDecimals(juce::MathConstants<float>::halfPi   * 0.5f,     2),
                    MathFunctions::roundToDecimals(juce::MathConstants<float>::halfPi,              2),
                    MathFunctions::roundToDecimals(juce::MathConstants<float>::pi       * 0.75f,    2),
                    MathFunctions::roundToDecimals(juce::MathConstants<float>::pi,                  2) }
                );
            }
        }

        setDisplayedUnit (state.displayInHz.getValue());
    }

    void setDisplayedUnit(bool hzIsTrueRadiansIsFalse)
    {
        const double sampleRate = processor.getSampleRate();
        const bool isLogarithmic = state.displayLogarithmic.getValue();

        std::vector<juce::String> labels;
        if (hzIsTrueRadiansIsFalse)
        {
            const int numTicks = isLogarithmic ? 9 : 3;
            for (int i = numTicks; i >= 0; i--)
            {
                juce::String label;

                const double freq = (sampleRate * 0.5) / (double)std::pow(2.0, i);
                if (freq >= 10000.0)
                    label = juce::String(std::round(freq / 1000.0), 0, false) + "k";
                else if (freq >= 1000.0)
                    label = juce::String(std::round(freq / 100.0) / 10.0, 1, false) + "k";
                else
                    label = juce::String(std::round(freq), 0, false);

                labels.emplace_back (label);
            }
        }
        else
        {
            if (isLogarithmic)
                labels = { "1/512", "1/256", "1/128", "1/64", "1/32", "1/16", "1/8", "1/4", "1/2", piString};
            else
                labels = { "1/4", "1/2", "3/4", piString};
        }

        for (auto* plot : juce::Array<Plot*>{&firstPlot, &secondPlot})
            plot->setXLabels (labels);

    }

    void rangeChanged(Plot* plot) override
    {
        const PlotType type = plot == &firstPlot ? state.firstPlotType.getValue()
                                                 : state.secondPlotType.getValue();
        updatePlotGrid (plot, type);
    }

    static void updatePlotGrid(Plot* plot, PlotType type)
    {
        const auto& range = plot->getYRange();
        std::vector<float> ticks;
        std::vector<juce::String> labels;

        if (type != PlotType::Phase)
            ticks = getGridYTicks (range, 8, 5.0f);
        else
            ticks = getGridYTicks (range, 8, juce::MathConstants<float>::pi);

        for (const auto& tick : ticks)
        {
            const juce::String label = tick == 0.0f ? "0" : juce::String(tick, 0, false);
            labels.emplace_back (label);
        }

        plot->setYTicks (ticks);
        plot->setYLabels (labels);
    }

    static std::vector<float> getGridYTicks(const MappedRange<float>& range, int numTicks, float maxStep)
    {
        const float span = range.end - range.start;

        if (span <= 0.0f)
            return {};

        float step = span / (float)numTicks;
        step = std::min(MathFunctions::roundToHighestMagnitude (step), maxStep);

        const float start = std::ceil((range.start / step)) * step;
        const float end = std::floor((range.end / step)) * step;
        const float truncatedSpan = end - start;

        float interval = truncatedSpan / (float)(numTicks + 1);
        interval = std::ceil(interval / step) * step;

        const float mag = std::pow(10.0f, std::ceil(std::log10(std::abs(interval))));
        const float scalar = 10.0f / mag;

        std::vector<float> ticks;

        float value = start;
        while (value <= end)
        {
            value = std::round(value * scalar) / scalar;
            ticks.emplace_back  (value);
            value += interval;
        }

        return ticks;
    }

    AudioPluginAudioProcessor& processor;
    State state;
    const juce::String piString         { juce::String (juce::CharPointer_UTF8 ("\xCF\x80")) }; // pi

    std::unique_ptr<PlotDataAttachment> plotAttachment;
    std::unique_ptr<PlotRangeAttachment> firstPlotRangeAttachment;
    std::unique_ptr<PlotRangeAttachment> secondPlotRangeAttachment;
};