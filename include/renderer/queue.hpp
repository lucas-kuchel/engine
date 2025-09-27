#pragma once

#include <cstdint>
#include <limits>

#include <vulkan/vulkan.h>

namespace renderer {
    class Instance;
    class Surface;

    enum class QueueType {
        PRESENT,
        TRANSFER,
        COMPUTE,
        RENDER,
    };

    struct QueueData {
        QueueType type;

        VkQueue queue = VK_NULL_HANDLE;

        std::uint32_t familyIndex = std::numeric_limits<std::uint32_t>::max();
        std::uint32_t queueIndex = std::numeric_limits<std::uint32_t>::max();
    };

    struct QueueCreateInfo {
        QueueType type;
        Instance& instance;
        Surface& surface;
    };

    class Queue {
    public:
        Queue();
        ~Queue();

        Queue(const Queue&) = delete;
        Queue(Queue&&) noexcept = default;

        Queue& operator=(const Queue&) = delete;
        Queue& operator=(Queue&&) noexcept = default;

        void create(const QueueCreateInfo& createInfo);

        QueueData& getData();

    private:
        QueueData data_;
    };
}