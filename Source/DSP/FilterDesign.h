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
        virtual void filterGainChanged([[maybe_unused]] FilterDesign* emitter) {}
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Listener)
    };

    void addListener (Listener* listener) { listeners.add (listener); }
    void removeListener (Listener* listener) { listeners.remove (listener); }

    //=======================================================================
    // Frequency response for a specific angle / frequency.
    struct Response
    {
        double angle { 0.0 };        // The frequency / angle in radians
        double magnitude { 0.0 };    // Magnitude response in amplitude (not dB)
        double phase { 0.0 };        // The unwrapped phase response
        double phaseDelay { 0.0 };
    };

    //=======================================================================
    // Set of feedback and feedforward coefficients
    struct CoefficientSet
    {
        std::vector<std::complex<double>> iirCoefs; // Feedback coefficients
        std::vector<std::complex<double>> firCoefs; // Feedforward coefficients
    };

    //=======================================================================
    /** Set the poles and zeros for the filter. This will internally cache them and recalculate
     *  the feedforward and feedback coefficients.
     *
     * @param poles         Vector of poles
     * @param zeros         Vector of zeros
     */
    void setPoleZeros(std::vector<std::complex<double>>& poles, std::vector<std::complex<double>>& zeros);

    //=======================================================================
    // Returns the feedback coefficients
    std::vector<std::complex<double>> getIIRCoefs() const;

    // Returns the feedforward coefficients
    std::vector<std::complex<double>> getFIRCoefs() const;

    /** Get the frequency response for a specific frequency / angle.
     *
     * @param angle         The frequency in radians
     * @param applyGain     If true, will apply the gained response.
     *                      If false, will return the normal filter response.
     */
    Response getFreqResponse(double angle, bool applyGain = true) const;

    static double getGroupDelay(Response& a, Response& b);

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

    void setGain(double gain);
    double getGain() const { return gain; }

    void setAutoNormalize(bool shouldAutoNormalize);
    bool isAutoNormalising() const { return autoNormalise; }

private:

    void updateNormalisationGain();

    //=======================================================================
    std::vector<std::complex<double>> poles;
    std::vector<std::complex<double>> zeros;
    CoefficientSet coefficients;

    double gain { 1.0f };
    bool autoNormalise { true };
    juce::ListenerList<Listener> listeners;
};