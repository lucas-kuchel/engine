#include <renderer/command_buffer.hpp>
#include <renderer/command_pool.hpp>
#include <renderer/device.hpp>
#include <renderer/queue.hpp>

namespace renderer {
    CommandPool CommandPool::create(const CommandPoolCreateInfo& createInfo) {
        CommandPool commandPool;

        auto& device = createInfo.device.device_;

        VkCommandPoolCreateInfo poolCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .pNext = nullptr,
            .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
            .queueFamilyIndex = createInfo.queue.familyIndex_,
        };

        if (vkCreateCommandPool(device, &poolCreateInfo, nullptr, &commandPool.commandPool_) != VK_SUCCESS) {
            commandPool.commandPool_ = nullptr;
        }
        else {
            commandPool.device_ = &createInfo.device;
            commandPool.queue_ = &createInfo.queue;
        }

        return commandPool;
    }

    void CommandPool::destroy(CommandPool& commandPool) {
        if (commandPool.commandPool_) {
            vkDestroyCommandPool(commandPool.device_->device_, commandPool.commandPool_, nullptr);

            commandPool.commandPool_ = nullptr;
        }
    }

    std::vector<CommandBuffer> CommandPool::allocateCommandBuffers(CommandPool& commandPool, std::uint32_t count) {
        VkCommandBufferAllocateInfo bufferAllocateInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .pNext = nullptr,
            .commandPool = commandPool.commandPool_,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = static_cast<std::uint32_t>(count),
        };

        std::vector<VkCommandBuffer> commandBuffers(count);
        std::vector<CommandBuffer> buffers;

        buffers.reserve(count);

        if (vkAllocateCommandBuffers(commandPool.device_->device_, &bufferAllocateInfo, commandBuffers.data()) != VK_SUCCESS) {
            return {};
        }

        for (std::size_t i = 0; i < count; i++) {
            buffers.push_back(CommandBuffer());

            auto& commandBuffer = buffers.back();

            commandBuffer.commandPool_ = &commandPool;

            buffers[i].commandBuffer_ = commandBuffers[i];
            buffers[i].capturing_ = false;
        }

        return buffers;
    }

    void CommandPool::destroyCommandBuffers(CommandPool& commandPool, const std::vector<CommandBuffer>& buffers) {
        std::vector<VkCommandBuffer> commandBuffers(buffers.size());

        for (std::size_t i = 0; i < buffers.size(); i++) {
            commandBuffers[i] = buffers[i].commandBuffer_;
        }

        vkFreeCommandBuffers(commandPool.device_->device_, commandPool.commandPool_, static_cast<std::uint32_t>(commandBuffers.size()), commandBuffers.data());
    }

    bool CommandPool::resetAllCommandBuffers(CommandPool& commandPool) {
        return vkResetCommandPool(commandPool.device_->device_, commandPool.commandPool_, 0) == VK_SUCCESS;
    }
}