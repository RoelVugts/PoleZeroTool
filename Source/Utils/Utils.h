#pragma once

#include <ranges>
#include <JuceHeader.h>

namespace Utils
{
    template<typename Container, typename T>
    static int indexOf(Container& container, const T& value)
    {
        auto it = std::ranges::find(container, value);
        return it != container.end() ? static_cast<int> (std::distance (container.begin(), it)) : -1;
    }

    static juce::String insertSpacesForEveryUpperCase(juce::String text)
    {
        for (int i = 1; i < text.length(); ++i)
        {
            if (juce::CharacterFunctions::isUpperCase(text[i]))
            {
                text = text.substring(0, i) + " " + text.substring(i);
                ++i;
            }
        }

        return text;
    }
}

