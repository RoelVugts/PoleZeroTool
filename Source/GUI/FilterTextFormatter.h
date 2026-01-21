#pragma once

#include <JuceHeader.h>
#include "../DSP/FilterDesign.h"

class FilterTextFormatter
{
public:

    // Returns the difference equation as a string (e.g. y[n] = 0.5x[n-1] - 0.3y[n-1])
    static juce::String differenceEquation (const FilterDesign& filter)
    {
        if (filter.getNormalisationGain() == 0.0f)
            return "y[n] = 0";

        const std::vector<std::complex<float>>& firCoefs = filter.getFIRCoefs();
        const std::vector<std::complex<float>>& iirCoefs = filter.getIIRCoefs();

        // Get degree of total polynomial
        const int polynomialDegree = (int) firCoefs.size() - (int) iirCoefs.size();

        juce::String differenceEquation = "y[n] = "; // Always start with this
        differenceEquation += coefsToFormulaString (firCoefs, false, polynomialDegree);
        differenceEquation += coefsToFormulaString (iirCoefs, true, polynomialDegree);

        return differenceEquation;
    }

    // Returns the difference equation as a string (e.g. y[n] = 0.5x[n-1] - 0.3y[n-1])
    static juce::String transferFunction   (const FilterDesign& filter)
    {
        const juce::String& numerator = polynomialEquation (filter, true);
        const juce::String& denumerator = polynomialEquation (filter, false);

        const juce::Font defaultFont(juce::FontOptions(11.0f));
        const int lineWidth = juce::GlyphArrangement::getStringWidthInt (defaultFont, "-");
        const int numeratorWidth = juce::GlyphArrangement::getStringWidthInt (defaultFont, numerator);
        const int denumeratorWidth = juce::GlyphArrangement::getStringWidthInt (defaultFont, denumerator);
        const int longestWidth = std::max(numeratorWidth, denumeratorWidth);
        const int numLines = longestWidth / lineWidth;

        juce::String lines("---");
        for (int i = 0; i < numLines; i++) {
            lines += "-";
        }

        return "            " + numerator + "\nH(z) = " + lines + "\n            " + denumerator;
    }

private:
    static juce::String coefsToFormulaString (const std::vector<std::complex<float>>& coefs, bool isOutput, int delayOffset)
    {
        juce::String text {};
        for (int i = 0; i < coefs.size(); i++)
        {
            // Don't show zero terms
            if (approximatelyEqual (coefs[i].real(), 0.0f)
                && approximatelyEqual (coefs[i].imag(), 0.0f))
                continue;

            // Skip first (a0) coefficient since that will be on left side of equation
            if (isOutput && i == 0)
                continue;

            // IIR is sign flipped
            const float real = isOutput ? -std::real(coefs[i]) : std::real(coefs[i]);
            const float imag = isOutput ? -std::imag(coefs[i]) : std::imag(coefs[i]);

            const juce::String io = isOutput ? "y" : "x";
            juce::String realDelay, imagDelay;

            const int delay = isOutput ? -i : delayOffset - i;
            const juce::String numString = delay > 0 ? "+" + juce::String(delay) : juce::String(delay);
            const juce::String index = delay == 0 ? "[n]" : "[n" + numString + "]";

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

        static juce::String polynomialEquation (const FilterDesign& filter, bool numerator)
    {
        if (numerator && filter.getNormalisationGain() == 0.0f) return "0";

        const auto& coefs = numerator ? filter.getFIRCoefs() : filter.getIIRCoefs();

        if (coefs.size() == 1) return juce::String(filter.getNormalisationGain());

        juce::String equation;

        for (int i = 0; i < coefs.size(); i++) {
            float real = std::real(coefs[i]);
            float imag = std::imag(coefs[i]);

            if (approximatelyEqual (real, 0.0f)) real = 0.0;
            if (approximatelyEqual (imag, 0.0f)) imag = 0.0;

            juce::String realNum(real);
            juce::String imagNum(imag);

            int power = (int)coefs.size() - (i+1);
            juce::String realPower = juce::String("z^") + juce::String(power);
            juce::String imagPower = juce::String("iz^") + juce::String(power);

            if (power == 1)
            {
                realPower = "z";
                imagPower = "iz";
            } else if (power == 0)
            {
                realPower = "";
                imagPower = "i";
            }

            if (realNum == "0" || realNum == "-0")  {
                realNum = "";
                realPower = "";
            } else if (realNum == "1" && power != 0 && equation.length() < 1) realNum = "";
            else if (realNum == "-1" && power != 0 && equation.length() < 1) realNum = " - ";
            else if (realNum == "1" && power != 0 && equation.length() >= 1) realNum = " + ";
            else if (realNum == "-1" && power != 0 && equation.length() >= 1) realNum = " - ";
            else if (real > 0.0 && equation.length() > 0) realNum = " + " + juce::String(real);
            else if (real > 0.0 && equation.length() < 1) realNum = juce::String(real);
            else if (real < 0.0) realNum = " - " + juce::String(std::abs(real));

            if (imagNum == "0" || imagNum == "-0") {
                imagNum = "";
                imagPower = "";
            } else if (imagNum == "1") imagNum = "";
            else if (imagNum == "-1") imagNum = " - ";
            else if (imag > 0.0) imagNum = " + " + juce::String(imag);
            else if (imag < 0.0) imagNum = " - " + juce::String(std::abs(imag));

            equation += realNum + realPower + imagNum + imagPower;
        }
        return equation;
    }
};