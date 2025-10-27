#pragma once

#include <cstdint>

namespace components {
    template <typename T>
    struct Proxy {
        std::uint64_t index = 0;
    };
}