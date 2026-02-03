#pragma once

#include "CircBuf.h"

#include <assert.h>
#include <complex>
#include <queue>
#include <vector>

/** A Direct form I filter using complex filter coefficients and data.*/
class ComplexFilter
{
public:

    std::complex<float> processSample(const float& input)
    {
        if (bypassed)
            return { input, 0.0f };

        std::complex<double> output(0.0, 0.0);

        // Write input into buffer
        firBuf.write (input);

        //===============================================================
        // FIR
        for (int k = 0; k < numFirCoefsUsed; k++)
            output += firCoefs[k] * firBuf.read (k);

        // IIR
        for (int k = 1; k < numIirCoefsUsed; k++)
            output += -iirCoefs[k] * iirBuf.read (k);

        //===============================================================
        // Make sure we don't blow up the speakers when a pole is placed outside the unit circle
        if (std::isnan (output.real()) || std::isinf (output.real()))
            output = { 0.0f, output.imag() };

        if (std::isnan (output.imag()) || std::isinf (output.imag()))
            output = { output.real(), 0.0f };

        output = { std::clamp(output.real(), -1.0, 1.0),
                      std::clamp(output.imag(), -1.0, 1.0) };

        //===============================================================
        // Write output back into output buf
        iirBuf.write (output);

        // Advance write pointers
        firBuf.incrementWriteIndex();
        iirBuf.incrementWriteIndex();

        return static_cast<std::complex<float>> (output);
    }

    void setCoefficients(const std::vector<std::complex<double>>& iir, const std::vector<std::complex<double>>& fir)
    {
        // Coefficients don't fit in pre-allocated array...
        assert(iir.size() <= maxCoefs && fir.size() <= maxCoefs);

        const int numFirCoefs = std::min((int)fir.size(), maxCoefs);
        const int numIirCoefs = std::min((int)iir.size(), maxCoefs);

        std::ranges::copy_n(fir.begin(), numFirCoefs, firCoefs.begin());
        std::ranges::copy_n(iir.begin(), numIirCoefs, iirCoefs.begin());
        numFirCoefsUsed = (int)fir.size();
        numIirCoefsUsed = (int)iir.size();
    }

    void setBypass(bool shouldBeBypassed)
    {
        bypassed = shouldBeBypassed;
    }

private:

    static constexpr int maxCoefs { 128 };

    std::array<std::complex<double>, maxCoefs> iirCoefs; // Feedback coefficients
    std::array<std::complex<double>, maxCoefs> firCoefs; // Feedforward coefficients

    CircBuf<std::complex<double>> iirBuf { maxCoefs };
    CircBuf<std::complex<double>> firBuf { maxCoefs };

    int numFirCoefsUsed { 0 };
    int numIirCoefsUsed { 0 };
    bool bypassed { false };
};