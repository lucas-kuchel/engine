#pragma once

#include <cstdint>
#include <vector>

#include <vulkan/vulkan.h>

namespace renderer {
    class Surface;
    class Semaphore;
    class Fence;
    class CommandBuffer;

    using Flags = std::uint32_t;

    // @brief Creation information for a queue
    struct QueueInfo {
        Flags flags;

        // @note Only necessary if the PRESENT flag is set
        Surface* surface;
    };

    struct QueueSubmitInfo {
        Fence& fence;

        std::vector<CommandBuffer> commandBuffers;
        std::vector<Semaphore> waits;
        std::vector<Semaphore> signals;
        std::vector<std::uint32_t> waitFlags;
    };

    class Queue {
    public:
        static bool submit(Queue& queue, const QueueSubmitInfo& submitInfo);

    private:
        VkQueue queue_ = nullptr;

        std::uint32_t familyIndex_;
        std::uint32_t queueIndex_;

        friend class Device;
        friend class CommandPool;
        friend class CommandBuffer;
        friend class Swapchain;
    };
}