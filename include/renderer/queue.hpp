#pragma once

#include <data/references.hpp>

#include <cstdint>
#include <vector>

#include <vulkan/vulkan.h>

namespace renderer {
    class Surface;
    class Semaphore;
    class Fence;
    class CommandBuffer;

    // @brief Queue capabilites required
    struct QueueFlags {
        enum {
            PRESENT = 1 << 0,
            TRANSFER = 1 << 1,
            COMPUTE = 1 << 2,
            RENDER = 1 << 3,
            PREFER_UNIQUE = 1 << 4,
        };
    };

    struct SubmitWaitFlags {
        enum {
            TOP_OF_PIPE = 1 << 0,
            DRAW_INDIRECT = 1 << 1,
            VERTEX_INPUT = 1 << 2,
            VERTEX_SHADER = 1 << 3,
            FRAGMENT_SHADER = 1 << 4,
            EARLY_FRAGMENT_TESTS = 1 << 5,
            LATE_FRAGMENT_TESTS = 1 << 6,
            COLOR_ATTACHMENT_OUTPUT = 1 << 7,
            TRANSFER = 1 << 8,
            BOTTOM_OF_PIPE = 1 << 9,
            HOST = 1 << 10,
            ALL_GRAPHICS = 1 << 11,
            ALL_COMMANDS = 1 << 12,
        };
    };

    // @brief Creation information for a queue
    struct QueueCreateInfo {
        std::uint32_t flags;

        // @note Only needs to be filled in if the PRESENT flag is set
        data::NullableReference<Surface> surface;
    };

    struct SubmitInfo {
        data::NullableReference<Fence> fence;
        std::vector<data::Reference<CommandBuffer>> commandBuffers;
        std::vector<data::Reference<Semaphore>> waits;
        std::vector<data::Reference<Semaphore>> signals;
        std::vector<std::uint32_t> waitFlags;
    };

    // @brief Represents a submission queue between the CPU and GPU
    // @note Not safe to copy
    class Queue {
    public:
        ~Queue();

        Queue(const Queue&) = delete;
        Queue(Queue&&) noexcept = default;

        Queue& operator=(const Queue&) = delete;
        Queue& operator=(Queue&&) noexcept = default;

        // @brief Submits work for processing on the GPU
        // @param The submission information
        void submit(const SubmitInfo& submitInfo);

        // @brief Provides the Vulkan VkQueue
        // @return The VkQueue
        [[nodiscard]] VkQueue& getVkQueue();

        // @brief Provides the Vulkan VkQueue
        // @return The VkQueue
        [[nodiscard]] const VkQueue& getVkQueue() const;

        // @brief Provides the Vulkan queue family index
        // @return The queue family index
        [[nodiscard]] std::uint32_t getVkFamilyIndex() const;

        // @brief Provides the Vulkan queue index
        // @return The queue index
        [[nodiscard]] std::uint32_t getVkQueueIndex() const;

    private:
        Queue() = default;

        VkQueue queue_ = VK_NULL_HANDLE;

        std::uint32_t familyIndex_;
        std::uint32_t queueIndex_;

        friend class Device;
    };
}