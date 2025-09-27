#pragma once

#include <cstdint>

namespace data {
    // @brief Represents a 2-dimensional area
    struct Extent2D {
        std::uint32_t width;
        std::uint32_t height;
    };
}