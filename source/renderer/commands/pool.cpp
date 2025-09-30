#include <renderer/commands/buffer.hpp>
#include <renderer/commands/pool.hpp>

#include <renderer/device.hpp>
#include <renderer/queue.hpp>

namespace renderer {
    CommandPool::CommandPool(const CommandPoolCreateInfo& createInfo)
        : device_(createInfo.device), queue_(createInfo.queue) {
        auto& device = createInfo.device.getVkDevice();

        VkCommandPoolCreateInfo poolCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .pNext = nullptr,
            .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
            .queueFamilyIndex = queue_->getVkFamilyIndex(),
        };

        if (vkCreateCommandPool(device, &poolCreateInfo, nullptr, &pool_) != VK_SUCCESS) {
            throw std::runtime_error("Error calling renderer::CommandPool::create(): Failed to create command pool");
        }
    }

    CommandPool::~CommandPool() {
        if (pool_ != VK_NULL_HANDLE) {
            vkDestroyCommandPool(device_->getVkDevice(), pool_, nullptr);
        }
    }

    std::vector<CommandBuffer> CommandPool::allocateCommandBuffers(const CommandBufferCreateInfo& createInfo) {
        auto& device = device_->getVkDevice();

        VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_MAX_ENUM;

        switch (createInfo.level) {
            case CommandBufferLevel::PRIMARY:
                level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
                break;

            case CommandBufferLevel::SECONDARY:
                level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
                break;
        }

        VkCommandBufferAllocateInfo bufferAllocateInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .pNext = nullptr,
            .commandPool = pool_,
            .level = level,
            .commandBufferCount = static_cast<std::uint32_t>(createInfo.count),
        };

        std::vector<VkCommandBuffer> commandBuffers(createInfo.count);
        std::vector<CommandBuffer> buffers;

        buffers.reserve(createInfo.count);

        if (vkAllocateCommandBuffers(device, &bufferAllocateInfo, commandBuffers.data()) != VK_SUCCESS) {
            throw std::runtime_error("Error calling renderer::CommandPool::allocateCommandBuffers(): Failed to allocate command buffers");
        }

        for (std::size_t i = 0; i < createInfo.count; i++) {
            buffers.push_back(CommandBuffer(*this));

            buffers[i].commandBuffer_ = commandBuffers[i];
            buffers[i].capturing_ = false;
        }

        return buffers;
    }

    void CommandPool::resetAllCommandBuffers() {
        if (vkResetCommandPool(device_->getVkDevice(), pool_, 0) != VK_SUCCESS) {
            throw std::runtime_error("Error calling renderer::CommandPool::reset(): Failed to reset all command buffers");
        }
    }

    VkCommandPool& CommandPool::getVkCommandPool() {
        return pool_;
    }

    const VkCommandPool& CommandPool::getVkCommandPool() const {
        return pool_;
    }
}