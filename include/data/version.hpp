#pragma once

#include <cstdint>

namespace data {
    // @brief Represents a 3-point version
    struct Version {
        std::uint32_t major = 0;
        std::uint32_t minor = 0;
        std::uint32_t patch = 0;
    };
}