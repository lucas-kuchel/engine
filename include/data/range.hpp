#pragma once

#include <type_traits>

namespace data {
    // @brief Represents a valid range of some kind
    // @tparam Any arithmetic type
    template <typename T>
    requires(std::is_arithmetic_v<T>)
    struct Range {
        T min = 0;
        T max = 0;
    };
}