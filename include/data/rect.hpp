#pragma once

#include <data/extent.hpp>
#include <data/position.hpp>

namespace data {
    // @brief Represents an offsettable extent
    // @tparam The type used for the offset. Any arithmetic type
    // @tparam The type used for the extent. Any arithmetic type
    template <typename T, typename U>
    requires(std::is_arithmetic_v<T> && std::is_arithmetic_v<U>)
    struct Rect2D {
        data::Position2D<T> offset;
        data::Extent2D<U> extent;
    };
}