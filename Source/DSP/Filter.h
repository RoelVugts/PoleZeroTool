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

        // FIR
        for (int k = 0; k < firCoefs.size(); k++)
            output += firCoefs[k] * firBuf.read (k);

        // IIR
        for (int k = 1; k < iirCoefs.size(); k++)
            output += -iirCoefs[k] * iirBuf.read (k);

        if (std::isnan (output.real()) || std::isinf (output.real()))
            output = { 0.0f, output.imag() };

        if (std::isnan (output.imag()) || std::isinf (output.imag()))
            output = { output.real(), 0.0f };

        output = { std::clamp(output.real(), -1.0, 1.0),
                      std::clamp(output.imag(), -1.0, 1.0) };

        // Write output back into output buf
        iirBuf.write (output);

        // Advance write pointers
        firBuf.incrementWriteIndex();
        iirBuf.incrementWriteIndex();

        // Make sure we don't blow up the speakers when a pole is placed outside the unit circle
        return static_cast<std::complex<float>> (output);
    }

    void setCoefficients(const std::vector<std::complex<double>>& iir, const std::vector<std::complex<double>>& fir)
    {
        //TODO: Avoid mem allocations
        iirCoefs = iir;
        firCoefs = fir;
    }

    void setBypass(bool shouldBeBypassed)
    {
        bypassed = shouldBeBypassed;
    }

private:
    std::vector<std::complex<double>> iirCoefs; // Feedback coefficients
    std::vector<std::complex<double>> firCoefs; // Feedforward coefficients

    CircBuf<std::complex<double>> iirBuf { 128 };
    CircBuf<std::complex<double>> firBuf { 128 };
    bool bypassed { false };
};