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

    float processSample(const float& input)
    {
        std::complex<double> output(0.0, 0.0);

        // Write input into buffer
        firBuf.write (input);

        // FIR
        for (int k = 0; k < firCoefs.size(); k++)
            output += firCoefs[k] * firBuf.read (k);

        // IIR
        for (int k = 1; k < iirCoefs.size(); k++)
            output += -iirCoefs[k] * iirBuf.read (k);

        // Write output back into output buf
        iirBuf.write (output);

        // Advance write pointers
        firBuf.incrementWriteIndex();
        iirBuf.incrementWriteIndex();

        return (float)output.real(); //TODO: Only real output for now
    }

    void process(const float* input, float* output, const int numSamples)
    {
        for (int i = 0; i < numSamples; i++)
            output[i] = processSample (input[i]);
    }

    void setCoefficients(const std::vector<std::complex<double>>& iir, const std::vector<std::complex<double>>& fir)
    {
        //TODO: Avoid mem allocations
        iirCoefs = iir;
        firCoefs = fir;
    }

private:
    std::vector<std::complex<double>> iirCoefs; // Feedback coefficients
    std::vector<std::complex<double>> firCoefs; // Feedforward coefficients

    CircBuf<std::complex<double>> iirBuf { 128 };
    CircBuf<std::complex<double>> firBuf { 128 };

};