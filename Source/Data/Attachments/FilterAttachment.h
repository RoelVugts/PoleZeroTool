#pragma once

#include <JuceHeader.h>

#include "../../DSP/Filter.h"
#include "../PoleZeroState.h"
#include "../../DSP/MathFunctions.h"

class FilterAttachment : private juce::AsyncUpdater
{
public:

    FilterAttachment (const PoleZeroState& settings)
        : state(settings)
    {
        state.points.setOnChildAdded ([this](juce::ValueTree&) {
            if (isUpdatePending())
                cancelPendingUpdate();

            triggerAsyncUpdate();
        });

        state.points.setOnChildRemoved ([this](juce::ValueTree&, int) {
            if (isUpdatePending())
                cancelPendingUpdate();

            triggerAsyncUpdate();
        });

        state.setOnAnyPropertyChanged ([this](juce::ValueTree&) {
            if (isUpdatePending())
                cancelPendingUpdate();

            triggerAsyncUpdate();
        });

        //=========================================================================
        FilterAttachment::handleAsyncUpdate();
    }

    bool shouldSwapCoefficients() const { return shouldSwap.load(); }
    std::vector<std::complex<float>> getIIRCoefs() const
    {
        return coefficients[activeBuffer].iirCoefs;
    }

    std::vector<std::complex<float>> getFIRCoefs() const
    {
        return coefficients[activeBuffer].firCoefs;
    }
    void markFilterAsUpdated() { shouldSwap.store(false); }

private:

    void handleAsyncUpdate() override
    {
        poles.clear();
        zeros.clear();

        for (int i = 0; i < state.points.size(); i++)
        {
            const auto point = state.points.getReference (i);
            const auto type = point.pointType.getValue();
            switch (type)
            {
                case PoZePlot::Point::Type::pole:
                    poles.emplace_back (point.value.getValue());
                    break;

                case PoZePlot::Point::Type::zero:
                    zeros.emplace_back (point.value.getValue());
                    break;
            }
        }

        coefficients[! activeBuffer].iirCoefs = MathFunctions::expandPolynomialFromRoots (poles);
        coefficients[! activeBuffer].firCoefs = MathFunctions::expandPolynomialFromRoots (zeros);
        std::cout << getDifferenceEquationText() << std::endl;
        activeBuffer = ! activeBuffer;
        shouldSwap = true;
    }

    juce::String getDifferenceEquationText() const
    {
        if (gain == 0.0)
            return "y[n] = 0";

        const std::vector<std::complex<float>>& coefficientList = coefficients[! activeBuffer].firCoefs;

        // Get degree of total polynomial
        const int polynomialDegree = (int) coefficients[! activeBuffer].firCoefs.size() - (int) coefficients[! activeBuffer].iirCoefs.size();

        juce::String differenceEquation = "y[n] = "; // We alwaus start here
        differenceEquation += coefsToFormulaString (coefficients[! activeBuffer].firCoefs, false, polynomialDegree);
        differenceEquation += coefsToFormulaString (coefficients[! activeBuffer].iirCoefs, true, polynomialDegree);

        return differenceEquation;
    }

    static juce::String coefsToFormulaString(const std::vector<std::complex<float>>& coefs, bool isIIR, int degree)
    {
        juce::String text {};
        for (int i = 0; i < coefs.size(); i++)
        {
            // Don't show zero terms
            if (approximatelyEqual (coefs[i].real(), 0.0f)
                && approximatelyEqual (coefs[i].imag(), 0.0f))
                continue;

            // Skip first (a0) coefficient since that will be on left side of equation
            if (isIIR && i == 0)
                continue;

            // IIR is sign flipped
            const float real = isIIR ? -std::real(coefs[i]) : std::real(coefs[i]);
            const float imag = isIIR ? -std::imag(coefs[i]) : std::imag(coefs[i]);

            const juce::String io = isIIR ? "y" : "x";
            juce::String realDelay, imagDelay;

            const int delay = isIIR ? -i : degree - i;
            const juce::String numString = delay > 0 ? "+" + juce::String(delay) : juce::String(delay);
            const juce::String index = (delay == 0) ? "[n]"
                                                       : "[n" + numString + "]";

            if (real != 0.0f)
                realDelay += io + index;

            if (imag != 0.0f)
                imagDelay += "i" + io + index;

            auto formatNumber = [](const float num) -> juce::String {
                if (approximatelyEqual (num, 0.0f))     return "";
                if (approximatelyEqual (num, 1.0f))     return " + ";
                if (approximatelyEqual (num, -1.0f))    return " - ";
                if (num > 0.0)                                return " + " + juce::String(num);
                if (num < 0.0)                                return " - " + juce::String(std::abs(num));

                return juce::String(num);
            };

            juce::String realNum = formatNumber(real);
            juce::String imagNum = formatNumber(imag);

            // Remove first plus sign
            if (i == 0 && real > 0.0) realNum = realNum.removeCharacters ("+").trim();

           text += realNum + realDelay + imagNum + imagDelay;
        }

        return text;
    }

    PoleZeroState state;

    std::vector<std::complex<float>> poles;
    std::vector<std::complex<float>> zeros;

    struct FilterCoefficients
    {
        std::vector<std::complex<float>> iirCoefs; // Feedback coefficients
        std::vector<std::complex<float>> firCoefs; // Feedforward coefficients
    };

    FilterCoefficients coefficients[2];
    int activeBuffer { 0 };
    std::atomic<bool> shouldSwap { false };
    float gain { 1.0f };
};