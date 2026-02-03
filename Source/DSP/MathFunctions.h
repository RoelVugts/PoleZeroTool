#pragma once

#include <algorithm>
#include <assert.h>
#include <complex>
#include <vector>

namespace MathFunctions
{
    /** Helper function to calculate the factorial of a number (eg 3! = 3 * 2 * 1 = 6).\n\n
     *  This function can only calculate factorials of numbers up to and including 20.
     *
     * @param n             The number to calculate factorial for
     */
    static int64_t factorial (int n)
    {
        // This function only works for factorials smaller than 21, since larger ones
        // will result in integer overflow...
        assert (n < 21);

        int64_t result = 1;
        for (int64_t i = 2; i <= n; ++i)
            result *= i;

        return result;
    }

    /** Function to calculate the sum of (complex) multiplication combinations. Say we have a list of 4 elements and we want to know the sum
     * of combinations of 2 elements. It calculates (a0a1 + a0a2 + a0a3 + a1a2 + a1a3 + a2a3)
     *
     * @param list          A list containing complex numbers
     * @param r             Amount of elements to make a combination with
     * @return
     */
    template <typename SampleType>
    static SampleType calculateSumOfCombinations (const std::vector<SampleType>& list, int r)
    {
        assert (r <= list.size()); // Impossible to make combinations with more elements than the list contains

        SampleType sum = 0.0;
        const int n = (int)list.size();

        // Calculate the total number of combinations
        const int totalCombinations = (int)(factorial (n) / (factorial (r) * factorial (n - r))); // nCr

        // Generate all combinations
        std::vector<bool> combination (n, false); // Combination mask (e.g. 0101 if r = 2)
        std::ranges::fill_n (combination.begin(), r, true); // start with 1100 (if r = 2)

        for (int i = 0; i < totalCombinations; ++i)
        {
            SampleType product = 1.0;
            for (int j = 0; j < n; ++j)
            {
                if (combination[j])
                    // Multiply the numbers in the current combination
                    product *= list[j];
            }
            sum += product; // add the product to the sum

            // Generate the next combination mask
            std::ranges::next_permutation (combination);
        }

        return sum;
    }

    /**
        Expands a polynomial from its roots and returns the coefficients.

        Given a list of roots, this function computes the polynomial coefficients
        such that:

            P(x) = (x - r0)(x - r1)(x - r2)...(x - rN)

        The coefficients are returned in ascending order of powers:

            coefficients[0] = 1
            coefficients[1] = -(r0 + r1 + ...)
            coefficients[2] = (sum of pairwise products)
            ...

        @param roots  The roots of the polynomial.
        @return       A vector containing the expanded polynomial coefficients.
    */
    template <typename SampleType>
    static std::vector<SampleType> expandPolynomialFromRoots (const std::vector<SampleType>& roots)
    {
        std::vector<SampleType> coefficients;

        // First coefficient is always 1.0
        coefficients.push_back (SampleType(1.0));

        // Start from second coefficient
        for (int i = 1; i <= (int)roots.size(); i++)
        {
            SampleType newCoefficient = calculateSumOfCombinations (roots, i);
            newCoefficient *= (i & 1) ? SampleType(-1.0) : SampleType(1.0); // Sign flips every coefficient
            coefficients.push_back (newCoefficient);
        }

        return coefficients;
    }

    /**
        Computes the angle of the vector difference between two complex points.

        Treats z1 and z2 as points in the complex plane and computes the angle
        of the vector pointing from z1 to z2.

        This is equivalent to:

            angle = atan2(imag(z2 - z1), real(z2 - z1))

        with additional logic to prevent phase wrapping discontinuities
        when working near the unit circle.

        @param z1  Start point in the complex plane.
        @param z2  End point in the complex plane.
        @return    Angle (in radians) of the difference vector.
    */
    template<typename SampleType>
    SampleType getAngleOfDifferenceVector(std::complex<SampleType> z1, std::complex<SampleType> z2)
    {
        SampleType deltaX = std::real(z2) - std::real(z1);
        SampleType deltaY = std::imag(z2) - std::imag(z1);

        // If the two points are at exactly the same position then there is no difference in angle.
        // Calling atan2(0,0) would lead to undefined behaviour.
        if (deltaX == 0.0 && deltaY == 0.0) return 0.0;

        SampleType angle = atan2(deltaY, deltaX);

        // Prevent phase wrapping
        // This makes sure the difference vector angle is between [-2pi, 2pi]
        // instead of [-pi, pi]
        if (std::arg(z2) > std::arg(z1) && std::abs(z1) <= 1.0)
        {
            if (std::imag(z1) > 0.0 && angle < 0.0)
                angle = angle + juce::MathConstants<SampleType>::twoPi;
        } else if (std::arg(z2) > std::arg(z1) && std::abs(z1) > 1.0)
        {
            if (std::imag(z1) > 0.0 && angle > 0.0)
                angle = angle - juce::MathConstants<SampleType>::twoPi;
        }

        return angle;
    }

    /**
        Rounds a value to a fixed number of decimal places.

        Example:
            roundToDecimals(3.14159, 2) → 3.14

        @param val          Value to round.
        @param numDecimals  Number of decimal places to keep.
        @return             Rounded value.
    */
    template<typename SampleType>
    SampleType roundToDecimals(SampleType val, int numDecimals)
    {
        const SampleType multiplier = std::pow(SampleType(10), SampleType(numDecimals));
        return std::round(val * multiplier) / multiplier;
    }

    /**
        Rounds a value to its most significant magnitude.

        This rounds the value to the nearest digit at the highest order of magnitude.

        Example:
            roundToHighestMagnitude(1234)  → 1000
            roundToHighestMagnitude(0.056) → 0.06

        @param val              Value to round.
        @param magnitudeOffset  Optional offset for shifting the rounding scale.
        @return                 Rounded value.
    */
    template<typename SampleType>
    SampleType roundToHighestMagnitude(SampleType val, int magnitudeOffset = 0)
    {
        if (val == SampleType(0.0))
            return SampleType(0.0);

        const SampleType mag = std::pow(SampleType(10.0), std::ceil(std::log10(std::abs(val)) - magnitudeOffset));
        const SampleType scalar = SampleType(10.0) / mag;
        return std::round(val * scalar) / scalar;
    }

    /**
        Computes the logarithm of a value with an arbitrary base.

        This is equivalent to:

            log_base(val) = log(val) / log(base)

        @param base Logarithmic base.
        @param val  Value to compute the logarithm of.
        @return     Logarithm of val with the given base.
    */
    template<typename SampleType>
    SampleType logBase(SampleType base, SampleType val)
    {
        return std::log(val) / std::log(base);
    }
}