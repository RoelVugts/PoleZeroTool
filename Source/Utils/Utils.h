#pragma once

#include <ranges>

namespace Utils
{
    template<typename Container, typename T>
    int indexOf(Container& container, const T& value)
    {
        auto it = std::ranges::find(container, value);
        return it != container.end() ? static_cast<int> (std::distance (container.begin(), it)) : -1;
    }
}

