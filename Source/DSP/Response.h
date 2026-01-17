#pragma once

#include <vector>
#include <complex>

#include "MathFunctions.h"

class Response
{
public:

    struct Sample
    {
        float angle;
        float phase;
        float magnitude;
    };

    void setData(const std::vector<std::complex<float>>& poles, const std::vector<std::complex<float>>& zeros)
    {

    }

    std::vector<Sample> response;


}