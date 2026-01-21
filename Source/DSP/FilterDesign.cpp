#include "FilterDesign.h"
#include "MathFunctions.h"

std::vector<std::complex<double>> FilterDesign::getIIRCoefs() const
{
    return coefficients.iirCoefs;
}

std::vector<std::complex<double>> FilterDesign::getFIRCoefs() const
{
    return coefficients.firCoefs;
}

void FilterDesign::setPoleZeros(std::vector<std::complex<double>>& poles_, std::vector<std::complex<double>>& zeros_)
{
    poles = poles_;
    zeros = zeros_;

    coefficients.iirCoefs = MathFunctions::expandPolynomialFromRoots (poles);
    coefficients.firCoefs = MathFunctions::expandPolynomialFromRoots (zeros);

    listeners.call([this](Listener& l) { l.filterCoefficientsChanged (this); });
}

FilterDesign::Response FilterDesign::getFreqResponse(const double angle) const
{
    const std::complex<double>& z = std::polar(1.0f, angle);

    const std::vector<std::complex<double>>& firCoefs = coefficients.firCoefs;
    const std::vector<std::complex<double>>& iirCoefs = coefficients.iirCoefs;

    std::complex<double> numerator(0.0f, 0.0f);
    std::complex<double> denumerator (0.0f, 0.0f);

    for (int i = 0; i < firCoefs.size(); i++) {
        int power = (int)firCoefs.size() - (i + 1);
        numerator += (std::pow(z, (double)power) * firCoefs[i]);
    }

    for (int i = 0; i < iirCoefs.size(); i++) {
        int power = (int)iirCoefs.size() - (i + 1);
        denumerator += std::pow(z, (double)power) * iirCoefs[i];
    }

    // Get the total (unwrapped) phase shift by accumulating the phase shift
    // from each pole and zero
    double phaseShift = 0.0f;

    for (auto zero : zeros)
        phaseShift += MathFunctions::getAngleOfDifferenceVector (zero, z);

    for (auto pole : poles)
        phaseShift -= MathFunctions::getAngleOfDifferenceVector (pole, z);

    std::complex<double> totalResponse = numerator / denumerator;
    double responseGain = std::abs(totalResponse);
    return FilterDesign::Response(std::arg(z), responseGain, phaseShift);

}

FilterDesign::Response FilterDesign::getMaxMagnitudeResponse(const int numAngles) const
{
    Response maxResponse {};
    const double angleStep = juce::MathConstants<double>::pi / (double)numAngles;

    for (int i = 0; i < numAngles; i++)
    {
        const double angle = (double) i * angleStep;
        maxResponse = std::max (getFreqResponse (angle), maxResponse, [] (const Response& a, const Response& b) {
            return a.magnitude > b.magnitude;
        });
    }

    return maxResponse;
}

FilterDesign::Response FilterDesign::getMaxPhaseResponse(const int numAngles) const
{
    Response maxResponse {};
    const double angleStep = juce::MathConstants<double>::pi / (double)numAngles;

    for (int i = 0; i < numAngles; i++)
    {
        const double angle = (double) i * angleStep;
        maxResponse = std::max (getFreqResponse (angle), maxResponse, [] (const Response& a, const Response& b) {
            return std::abs(a.phase) > std::abs(b.phase);
        });
    }

    return maxResponse;
}

