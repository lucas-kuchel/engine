#pragma once

#include <cstdint>

namespace engine::components {
    template <typename T>
    struct Proxy {
        std::uint64_t index = 0;
    };
}