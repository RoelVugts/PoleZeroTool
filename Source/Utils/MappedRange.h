#pragma once

#include "../DSP/MathFunctions.h"

#include <functional>

/** Maps a normalized value to a specified range. This is similar to juce::NormalisableRange
 *  but this also allows values to be mapped outside of the range.
 */
template<typename SampleType>
class MappedRange {
public:
    using ValueRemapFn = std::function<SampleType(SampleType s, SampleType e, SampleType v)>;

    MappedRange() = default;

    MappedRange(SampleType minValue, SampleType maxValue) : start(minValue), end(maxValue)
    {
        convertFrom0To1Func = [](SampleType s, SampleType e, SampleType v) -> SampleType { return v * (e - s) + s; };
        convertTo0To1Func = [](SampleType s, SampleType e, SampleType v) -> SampleType { return (v - s) / (e - s); };
    }

    MappedRange(SampleType minValue, SampleType maxValue, ValueRemapFn from0To1Fn, ValueRemapFn to0To1Fn)
    : start(minValue)
    , end(maxValue)
    , convertFrom0To1Func(std::move(from0To1Fn))
    , convertTo0To1Func(std::move(to0To1Fn))
    {

    }

    SampleType convertTo0to1(SampleType val) const
    {
        return convertTo0To1Func(start, end, val);
    }

    SampleType convertFrom0to1(SampleType val) const
    {
        return convertFrom0To1Func(start, end, val);
    }


    static MappedRange<SampleType> createExponentialRange(SampleType start, SampleType end, SampleType curve = SampleType(10.0))
    {
        // Curve must be greater than 0
        jassert(curve > 0.0);

        MappedRange<SampleType> range;
        range.start = start;
        range.end = end;

        curve = std::pow(curve, SampleType(3.0));
        range.convertFrom0To1Func = [curve](SampleType start, SampleType end, SampleType v) {
            return ((std::pow(curve + SampleType(1.0), v) - SampleType(1.0)) / curve) * (end - start) + start;
        };

        range.convertTo0To1Func = [curve](SampleType start, SampleType end, SampleType v) {
            return MathFunctions::logBase (curve + 1, ((v - start) / (end - start)) * curve + 1);
        };

        return range;
    }

    SampleType start { SampleType(0.0) };
    SampleType end { SampleType(0.0) };

private:
    ValueRemapFn convertFrom0To1Func { nullptr };
    ValueRemapFn convertTo0To1Func { nullptr };
};