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
        const int n = list.size();

        // Calculate the total number of combinations
        const int totalCombinations = factorial (n) / (factorial (r) * factorial (n - r)); // nCr

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

    template<typename SampleType>
    SampleType getAngleOfDifferenceVector(std::complex<SampleType> z1, std::complex<SampleType> z2)
    {
        SampleType deltaX = std::real(z2) - std::real(z1);
        SampleType deltaY = std::imag(z2) - std::imag(z1);

        // If the two points are at exactly the same position then there is no difference in angle.
        // Calling atan2(0,0) would lead to undefined behaviour.
        if (deltaX == 0.0 && deltaY == 0.0) return 0.0;

        SampleType angle = atan2(deltaY, deltaX);

        return angle;
    }
}