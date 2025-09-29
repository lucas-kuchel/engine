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

    // @brief The purpose of the queue
    enum class QueueType {
        PRESENT,
        TRANSFER,
        COMPUTE,
        RENDER,
    };

    // @brief Creation information for a queue
    struct QueueCreateInfo {
        QueueType type;

        // @note Only needs to be filled in if creating a PRESENT queue
        data::NullableReference<Surface> surface;
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
        // @param The list of command buffers to process
        // @param The semaphore to signal GPU availability
        // @param The semaphore to signal GPU operation completion
        // @param The fence to indicate if operations are active
        void submit(const std::vector<data::Reference<CommandBuffer>>& commandBuffers, const std::vector<data::Reference<Semaphore>>& available, const std::vector<data::Reference<Semaphore>>& finished, Fence& inFlight);

        // @brief Provides the queue type
        // @return The queue type
        [[nodiscard]] QueueType getType() const;

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

        QueueType type_;

        VkQueue queue_ = VK_NULL_HANDLE;

        std::uint32_t familyIndex_;
        std::uint32_t queueIndex_;

        friend class Device;
    };
}