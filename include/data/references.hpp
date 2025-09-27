#pragma once

#include <functional>
#include <vector>

namespace data {
    template <typename T>
    using ReferenceList = std::vector<std::reference_wrapper<T>>;
}