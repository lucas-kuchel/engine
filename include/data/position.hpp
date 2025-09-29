#pragma once

#include <type_traits>

namespace data {
    // @brief Represents a 2-dimensional position
    // @tparam Any arithmetic type
    template <typename T>
    requires(std::is_arithmetic_v<T>)
    struct Position2D {
        T x = 0;
        T y = 0;
    };

    // @brief Represents a 3-dimensional position
    // @tparam Any arithmetic type
    template <typename T>
    requires(std::is_arithmetic_v<T>)
    struct Position3D {
        T x = 0;
        T y = 0;
        T z = 0;
    };
}