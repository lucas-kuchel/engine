#pragma once

#include <cstdint>

namespace data {
    // @brief Represents a 2-dimensional area
    struct Extent2D {
        std::uint32_t width;
        std::uint32_t height;
    };

    // @brief Represents a 3-dimensional area
    struct Extent3D {
        std::uint32_t width;
        std::uint32_t height;
        std::uint32_t depth;
    };
}