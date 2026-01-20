#pragma once

#include <JuceHeader.h>

#include <complex>

class FilterDesign
{
public:

    //=======================================================================
    class Listener
    {
    public:
        Listener() = default;
        virtual ~Listener() = default;
        virtual void filterCoefficientsChanged ([[maybe_unused]] FilterDesign* emitter) {}
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Listener)
    };

    void addListener (Listener* listener) { listeners.add (listener); }
    void removeListener (Listener* listener) { listeners.remove (listener); }

    //=======================================================================
    // Frequency response for a specific angle / frequency.
    struct Response
    {
        float angle { 0.0f };        // The frequency / angle in radians
        float magnitude { 0.0f };    // Magnitude response in amplitude (not dB)
        float phase { 0.0f };        // The unwrapped phase response
    };

    //=======================================================================
    // Set of feedback and feedforward coefficients
    struct CoefficientSet
    {
        std::vector<std::complex<float>> iirCoefs; // Feedback coefficients
        std::vector<std::complex<float>> firCoefs; // Feedforward coefficients
    };

    //=======================================================================
    /** Set the poles and zeros for the filter. This will internally cache them and recalculate
     *  the feedforward and feedback coefficients.
     *
     * @param poles         Vector of poles
     * @param zeros         Vector of zeros
     */
    void setPoleZeros(std::vector<std::complex<float>>& poles, std::vector<std::complex<float>>& zeros);

    //=======================================================================
    // Returns the feedback coefficients
    std::vector<std::complex<float>> getIIRCoefs() const;

    // Returns the feedforward coefficients
    std::vector<std::complex<float>> getFIRCoefs() const;

    /** Get the frequency response for a specific frequency / angle.
     *
     * @param angle         The frequency in radians
     */
    Response getFreqResponse(float angle) const;

    /** Returns the frequency response with the highest
     *  magnitude. It finds the highest magnitude by testing multiple frequencies.
     *  This is a bit of a brute force way but there is (afaik) no direct formula to get max(|H(z)|)
     *
     *  @param numAngles            The number of angles / frequencies to test.
     *                              The interval between frequencies will be pi / numAngles.
     *
     *  @returns                    The frequency response with the highest magnitude.
     */
    Response getMaxMagnitudeResponse(int numAngles = 256) const;

    /** Returns the frequency response with the largest absolute phas shift.
     *  It finds it by testing multiple frequencies.
     *  This is a bit of a brute force way but there is (afaik) no direct formula to get <(H(z))
     *
     *  @param numAngles            The number of angles / frequencies to test.
     *                              The interval between frequencies will be pi / numAngles.
     *
     *  @returns                    The frequency response with the largest absolute phase shift.
     */
    Response getMaxPhaseResponse(int numAngles = 256) const;

    //=======================================================================
    // Returns the difference equation as a string (e.g. y[n] = 0.5x[n-1] - 0.3y[n-1])
    juce::String getDifferenceEquationText() const;

private:

    //=======================================================================
    static juce::String coefsToFormulaString(const std::vector<std::complex<float>>& coefs, bool isIIR, int degree);

    //=======================================================================
    std::vector<std::complex<float>> poles;
    std::vector<std::complex<float>> zeros;
    CoefficientSet coefficients;

    float gain { 1.0f };
    juce::ListenerList<Listener> listeners;
};