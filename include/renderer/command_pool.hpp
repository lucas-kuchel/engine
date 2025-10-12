#pragma once

#include <cstdint>
#include <vector>

#include <vulkan/vulkan.h>

namespace renderer {
    class Device;
    class Queue;
    class CommandBuffer;

    // @brief Creation information for a command pool
    struct CommandPoolCreateInfo {
        Device& device;
        Queue& queue;
    };

    class CommandPool {
    public:
        static CommandPool create(const CommandPoolCreateInfo& createInfo);
        static void destroy(CommandPool& commandPool);

        static std::vector<CommandBuffer> allocateCommandBuffers(CommandPool& commandPool, std::uint32_t count);

        static void destroyCommandBuffers(CommandPool& commandPool, const std::vector<CommandBuffer>& buffers);
        static bool resetAllCommandBuffers(CommandPool& commandPool);

    private:
        VkCommandPool commandPool_ = nullptr;
        Device* device_ = nullptr;
        Queue* queue_ = nullptr;
    };
}