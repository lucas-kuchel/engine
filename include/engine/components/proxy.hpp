#pragma once

#include <functional>

namespace engine::components {
    template <typename T>
    struct Proxy {
        std::reference_wrapper<T> value;
    };
}