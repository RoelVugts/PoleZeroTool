#pragma once

#include <JuceHeader.h>

#include "../../Data/Attachments/DragBoxAttachment.h"
#include "../../Data/Attachments/PlotAttachment.h"
#include "../../Data/Attachments/ResponsePlotAttachment.h"
#include "../Components/Plot.h"
#include "../DSP/MathFunctions.h"
#include "../LookAndFeel.h"

class ResponsePlotSection : public juce::Component, private Plot::Listener
{
public:

    ResponsePlotSection(AudioPluginAudioProcessor& p)
        : processor(p), state(p.state)
    {
        for (auto* plot : juce::Array<Plot*>{&magnitudePlot, &phasePlot, &groupDelayPlot})
        {
            plot->addListener (this);
            addAndMakeVisible (plot);
        }

        //==================================================================================================
        state.setOnPropertyChanged (State::IDs::displayInDB, [this]() {
            const bool shouldDisplayInDecibels = state.displayInDB.getValue();
            const float start = state.magnitudePlotRange.getValue().getStart();
            const float end = state.magnitudePlotRange.getValue().getEnd();
            if (shouldDisplayInDecibels)
            {
                magnitudePlot.setMinMaxRange (-100.0f, 100.0f);
                magnitudePlot.setRange ({ juce::Decibels::gainToDecibels (start),juce::Decibels::gainToDecibels (end)}, true );
            }
            else
            {
                magnitudePlot.setMinMaxRange (0.0f, 100.0f);
                magnitudePlot.setRange ({ start,end }, true );
            }
        }, true);

        //==================================================================================================
        plotAttachment = std::make_unique<ResponsePlotAttachment> (p.state, p.filterDesign, magnitudePlot, phasePlot, groupDelayPlot);
        phasePlotAttachment = std::make_unique<PlotAttachment>(phasePlot, state.phasePlotRange);
        groupDelayPlotAttachment = std::make_unique<PlotAttachment>(groupDelayPlot, state.groupDelayPlotRange);

        //==================================================================================================
        state.setOnPropertyChanged (State::IDs::displayGroupDelay, [this]() {
            if (state.displayGroupDelay.getValue())
            {
                groupDelayPlot.setVisible (true);
                phasePlot.setVisible (false);
            }
            else
            {
                groupDelayPlot.setVisible (false);
                phasePlot.setVisible (true);
            }
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
        magnitudePlot.setBounds (magPlotArea.toNearestInt());

        bounds.removeFromTop (LAF::Layout::defaultSpacing);
        auto phasePlotArea = bounds;
        phasePlot.setBounds (phasePlotArea.toNearestInt());
        groupDelayPlot.setBounds (phasePlotArea.toNearestInt());
    }

    Plot magnitudePlot { "Magnitude", -100.0f, 100.0f };
    Plot phasePlot { "Phase", -1000.0f, 1000.0f };
    Plot groupDelayPlot { "Group Delay", -1000.0f, 1000.0f };
    std::unique_ptr<ResponsePlotAttachment> plotAttachment;

private:

    void setLogDomain(bool shouldDisplayLogarithmic)
    {
        if (shouldDisplayLogarithmic)
        {
            MappedRange<float> logRange = MappedRange<float>::createExponentialRange (0.0f, juce::MathConstants<float>::pi);

            for (auto* plot : juce::Array<Plot*>{&magnitudePlot, &phasePlot, &groupDelayPlot})
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
            for (auto* plot : juce::Array<Plot*>{&magnitudePlot, &phasePlot, &groupDelayPlot})
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

        for (auto* plot : juce::Array<Plot*>{&magnitudePlot, &phasePlot, &groupDelayPlot})
            plot->setXLabels (labels);

    }

    void rangeChanged(Plot* plot) override
    {
        const auto& range = plot->getYRange();
        std::vector<float> ticks;
        std::vector<juce::String> labels;

        if (plot == &magnitudePlot || &groupDelayPlot)
            ticks = getGridYTicks (range, 8, 5.0f);
        else if (plot == &phasePlot)
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

    std::unique_ptr<PlotAttachment> magPlotAttachment;
    std::unique_ptr<PlotAttachment> phasePlotAttachment;
    std::unique_ptr<PlotAttachment> groupDelayPlotAttachment;
};