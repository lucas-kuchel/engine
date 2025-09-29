#pragma once

#include <type_traits>

namespace data {
    // @brief Represents a 2-dimensional area
    // @tparam Any arithmetic type
    template <typename T>
    requires(std::is_arithmetic_v<T>)
    struct Extent2D {
        T width = 0;
        T height = 0;
    };

    // @brief Represents a 3-dimensional area
    // @tparam Any arithmetic type
    template <typename T>
    requires(std::is_arithmetic_v<T>)
    struct Extent3D {
        T width = 0;
        T height = 0;
        T depth = 0;
    };
}