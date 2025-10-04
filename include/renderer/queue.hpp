#pragma once

#include <renderer/resources/config.hpp>

#include <data/references.hpp>

#include <vector>

#include <vulkan/vulkan.h>

namespace renderer {
    class Surface;
    class Semaphore;
    class Fence;
    class CommandBuffer;

    // @brief Creation information for a queue
    struct QueueCreateInfo {
        Flags flags;

        // @note Must provide only if the PRESENT flag is set
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