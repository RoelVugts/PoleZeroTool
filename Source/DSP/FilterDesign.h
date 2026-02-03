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

    /** Returns the group delay. The group delay is the derivative of the (negative) phase shift.
     *  Therefore we need to calculate the difference between two frequencies.
     *
     *  @param a            The lower frequency
     *  @param b            The higher frequency
     *
     *  @return            The group delay in samples
     */
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

    //=======================================================================
    /** Set the overall filter gain.
     *
     * @param gain          Gain in amplitude
     */
    void setGain(double gain);

    // Returns the gain applied to the filter.
    double getGain() const { return gain; }

    /** Enables automatic normalisation. If enabled the gain will be set so the highest
     *  filter peak will be at unity gain.
     */
    void setAutoNormalize(bool shouldAutoNormalise);

    // Returns true if automatic normalisation is enabled.
    bool isAutoNormalising() const { return autoNormalise; }

private:

    void updateNormalisationGain();

    //=======================================================================
    std::vector<std::complex<double>> poles;
    std::vector<std::complex<double>> zeros;
    CoefficientSet coefficients;

    double gain { 1.0f };
    bool autoNormalise { true };
    juce::ThreadSafeListenerList<Listener> listeners;
};