#pragma once

namespace renderer {
    // @brief Describes how allocated memory can be accessed
    enum class MemoryType {
        // @brief Visible to the CPU; changes to data will not be applied until flushed
        HOST_VISIBLE,

        // @brief Not visible to the CPU; most efficient for commonly used memory on the device
        DEVICE_LOCAL,
    };

    struct BufferCreateInfo {
    };
}