#include "FilterDesign.h"
#include "MathFunctions.h"

std::vector<std::complex<float>> FilterDesign::getIIRCoefs() const
{
    return coefficients.iirCoefs;
}

std::vector<std::complex<float>> FilterDesign::getFIRCoefs() const
{
    return coefficients.firCoefs;
}

void FilterDesign::setPoleZeros(std::vector<std::complex<float>>& poles_, std::vector<std::complex<float>>& zeros_)
{
    poles = poles_;
    zeros = zeros_;

    coefficients.iirCoefs = MathFunctions::expandPolynomialFromRoots (poles);
    coefficients.firCoefs = MathFunctions::expandPolynomialFromRoots (zeros);

    listeners.call([this](Listener& l) { l.filterCoefficientsChanged (this); });
}


juce::String FilterDesign::getDifferenceEquationText() const
{
    if (gain == 0.0)
        return "y[n] = 0";

    const std::vector<std::complex<float>>& firCoefs = coefficients.firCoefs;
    const std::vector<std::complex<float>>& iirCoefs = coefficients.iirCoefs;

    // Get degree of total polynomial
    const int polynomialDegree = (int) firCoefs.size() - (int) iirCoefs.size();

    juce::String differenceEquation = "y[n] = "; // Always start with this
    differenceEquation += coefsToFormulaString (firCoefs, false, polynomialDegree);
    differenceEquation += coefsToFormulaString (iirCoefs, true, polynomialDegree);

    return differenceEquation;
}

juce::String FilterDesign::coefsToFormulaString(const std::vector<std::complex<float>>& coefs, bool isIIR, int degree)
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

FilterDesign::Response FilterDesign::getFreqResponse(const float angle) const
{
    const std::vector<std::complex<float>>& firCoefs = coefficients.firCoefs;
    const std::vector<std::complex<float>>& iirCoefs = coefficients.iirCoefs;

    std::complex<float> numerator(0.0f, 0.0f);
    std::complex<float> denumerator (0.0f, 0.0f);

    const std::complex<float> z = std::polar(1.0f, angle);

    for (int i = 0; i < firCoefs.size(); i++) {
        int power = (int)firCoefs.size() - (i + 1);
        numerator += (std::pow(z, (float)power) * firCoefs[i]);
    }

    for (int i = 0; i < iirCoefs.size(); i++) {
        int power = (int)iirCoefs.size() - (i + 1);
        denumerator += std::pow(z, (float)power) * iirCoefs[i];
    }

    // Get the total (unwrapped) phase shift by accumulating the phase shift
    // from each pole and zero
    float phaseShift = 0.0f;

    for (auto zero : zeros)
        phaseShift += MathFunctions::getAngleOfDifferenceVector (zero, z);

    for (auto pole : poles)
        phaseShift -= MathFunctions::getAngleOfDifferenceVector (pole, z);


    std::complex<float> totalResponse = numerator / denumerator;
    float responseGain = std::abs(totalResponse);
    FilterDesign::Response freqResponse(std::arg(z), responseGain, phaseShift);

    return freqResponse;
}

FilterDesign::Response FilterDesign::getMaxMagnitudeResponse(const int numAngles) const
{
    Response maxResponse {};
    const float angleStep = juce::MathConstants<float>::pi / (float)numAngles;

    for (int i = 0; i < numAngles; i++)
    {
        const float angle = (float) i * angleStep;
        maxResponse = std::max (getFreqResponse (angle), maxResponse, [] (const Response& a, const Response& b) {
            return a.magnitude > b.magnitude;
        });
    }

    return maxResponse;
}

FilterDesign::Response FilterDesign::getMaxPhaseResponse(const int numAngles) const
{
    Response maxResponse {};
    const float angleStep = juce::MathConstants<float>::pi / (float)numAngles;

    for (int i = 0; i < numAngles; i++)
    {
        const float angle = (float) i * angleStep;
        maxResponse = std::max (getFreqResponse (angle), maxResponse, [] (const Response& a, const Response& b) {
            return std::abs(a.phase) > std::abs(b.phase);
        });
    }

    return maxResponse;
}

