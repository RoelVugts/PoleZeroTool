#pragma once

#include <JuceHeader.h>
#include "../DSP/FilterDesign.h"

class FilterTextFormatter
{
public:

    enum class PolynomialDomain
    {
        DifferenceEquation,
        ZDomain
    };

    // Returns the difference equation as a string (e.g. y[n] = 0.5x[n-1] - 0.3y[n-1])
    static juce::String differenceEquation (const FilterDesign& f)
    {
        if (approximatelyEqual (f.getGain(), 0.0))
            return "y[n] = 0";

        const int degree = (int)f.getFIRCoefs().size() - (int)f.getIIRCoefs().size();

        return "y[n] = "
            + formatCoefficients (f.getFIRCoefs(), PolynomialDomain::DifferenceEquation, false, degree)
            + formatCoefficients (f.getIIRCoefs(), PolynomialDomain::DifferenceEquation, true,  degree);
    }

    /** Returns the trasnfer fucnction as a string (e.g.\n
     *            0.5z^2 + 0.3z
     * \n H(z) = ---------------\n
     *            0.2z^2 - 0.87z
     */
    static juce::String transferFunction   (const FilterDesign& filter)
    {
        if (approximatelyEqual (filter.getGain(), 0.0))
            return "H(z) = 0";

        const juce::String& numerator = formatCoefficients (filter.getFIRCoefs(), PolynomialDomain::ZDomain,false,0);
        const juce::String& denumerator = formatCoefficients (filter.getIIRCoefs(), PolynomialDomain::ZDomain, false,0);

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
    static juce::String formatCoefficients (const std::vector<std::complex<double>>& coefs, PolynomialDomain domain, bool isOutput, int delayOffset)
    {
        if (domain == PolynomialDomain::ZDomain)
            if (coefs.size() == 1)
                return "1";

        juce::String result;

        for (size_t i = 0; i < coefs.size(); ++i)
        {
            if (approximatelyEqual (coefs[i].real(), 0.0)
             && approximatelyEqual (coefs[i].imag(), 0.0))
                continue;

            if (domain == PolynomialDomain::DifferenceEquation && isOutput && i == 0)
                continue;

            const double real = (isOutput ? -1.0f : 1.0f) * std::real(coefs[i]);
            const double imag = (isOutput ? -1.0f : 1.0f) * std::imag(coefs[i]);

            result += formatComplexTerm (real, imag, domain, (int)i, (int)coefs.size(), delayOffset, isOutput, result.isEmpty() && ! isOutput);
        }

        return result;
    }

    static juce::String formatComplexTerm (double real, double imag, PolynomialDomain domain, int index, int size, int delayOffset, bool isOutput, bool isFirst)
    {
        auto formatNumber = [] (double v, bool first) -> juce::String
        {
            if (approximatelyEqual (v, 0.0)) return juce::String{};
            if (approximatelyEqual (v, 1.0)) return first ? "" : " + ";
            if (approximatelyEqual (v, -1.0)) return " - ";
            if (v > 0.0f) return (first ? "" : " + ") + juce::String(v);
            return " - " + juce::String(std::abs(v));
        };

        juce::String term;

        const juce::String realNum = formatNumber (real, isFirst);
        const juce::String imagNum = formatNumber (imag, false);

        term += realNum;
        if (! approximatelyEqual (real, 0.0))
            term += getUnitString (domain, index, size, delayOffset, isOutput, false);

        term += imagNum;
        if (! approximatelyEqual (imag, 0.0))
            term += getUnitString (domain, index, size, delayOffset, isOutput, true);

        return term;
    }

    static juce::String getUnitString (PolynomialDomain domain, int i, int size, int delayOffset, bool isOutput, bool imaginary)
    {
        if (domain == PolynomialDomain::DifferenceEquation)
        {
            const int delay = isOutput ? -i : delayOffset - i;
            const juce::String idx = delay == 0 ? "[n]" : "[n" + juce::String(delay > 0 ? "+" : "") + juce::String(delay) + "]";
            return imaginary ? juce::String("i") + (isOutput ? "y" : "x") + idx
                             : juce::String((isOutput ? "y" : "x")) + idx;
        }

        // Z-domain
        const int power = size - (i + 1);
        if (power == 0) return imaginary ? "i" : "";
        if (power == 1) return imaginary ? "iz" : "z";

        return imaginary ? "iz^" + juce::String(power)
                         : "z^" + juce::String(power);
    }
};