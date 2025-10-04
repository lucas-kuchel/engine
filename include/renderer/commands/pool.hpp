#pragma once

#include <data/references.hpp>

#include <cstdint>
#include <vector>

#include <vulkan/vulkan.h>

namespace renderer {
    class Device;
    class Queue;
    class CommandBuffer;

    // @brief Creation information for a command buffer
    struct CommandBufferCreateInfo {
        std::uint32_t count;
    };

    // @brief Creation information for a command pool
    struct CommandPoolCreateInfo {
        Device& device;
        Queue& queue;
    };

    // @brief Allocates command buffers for a queue
    // @note Not safe to copy
    class CommandPool {
    public:
        CommandPool(const CommandPoolCreateInfo& createInfo);
        ~CommandPool();

        CommandPool(const CommandPool&) = delete;
        CommandPool(CommandPool&&) noexcept = default;

        CommandPool& operator=(const CommandPool&) = delete;
        CommandPool& operator=(CommandPool&&) noexcept = default;

        // @brief Allocates command buffers to be used for command submission to queues
        // @param Creation info for the buffers
        // @return The created command buffers
        [[nodiscard]] std::vector<CommandBuffer> allocateCommandBuffers(const CommandBufferCreateInfo& createInfo);

        // @brief Resets all command buffers in the pool
        void resetAllCommandBuffers();

        // @brief Provides the Vulkan VkCommandPool
        // @return The VkCommandPool
        [[nodiscard]] VkCommandPool& getVkCommandPool();

        // @brief Provides the Vulkan VkCommandPool
        // @return The VkCommandPool
        [[nodiscard]] const VkCommandPool& getVkCommandPool() const;

    private:
        VkCommandPool pool_;

        data::Reference<Device> device_;
        data::Reference<Queue> queue_;
    };
}