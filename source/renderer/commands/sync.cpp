#include <renderer/commands/sync.hpp>

#include <renderer/device.hpp>

namespace renderer {
    Semaphore::Semaphore(const SemaphoreCreateInfo& createInfo)
        : device_(createInfo.device) {
        auto& device = device_->getVkDevice();

        VkSemaphoreCreateInfo semaphoreCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
        };

        if (vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &semaphore_) != VK_SUCCESS) {
            throw std::runtime_error("Error constructing renderer::Semaphore: Failed to create semaphore");
        }
    }

    Semaphore::~Semaphore() {
        auto& device = device_->getVkDevice();

        if (semaphore_ != VK_NULL_HANDLE) {
            vkDestroySemaphore(device, semaphore_, nullptr);
        }
    }

    VkSemaphore& Semaphore::getVkSemaphore() {
        return semaphore_;
    }

    const VkSemaphore& Semaphore::getVkSemaphore() const {
        return semaphore_;
    }

    Fence::Fence(const FenceCreateInfo& createInfo)
        : device_(createInfo.device) {
        VkFenceCreateInfo fenceCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
            .pNext = nullptr,
            .flags = VK_FENCE_CREATE_SIGNALED_BIT,
        };

        if (vkCreateFence(device_->getVkDevice(), &fenceCreateInfo, nullptr, &fence_) != VK_SUCCESS) {
            throw std::runtime_error("Error constructing renderer::Fence: Failed to create fence");
        }
    }

    Fence::~Fence() {
        if (fence_ != VK_NULL_HANDLE) {
            vkDestroyFence(device_->getVkDevice(), fence_, nullptr);
        }
    }

    void Fence::await() {
        if (vkWaitForFences(device_->getVkDevice(), 1, &fence_, VK_TRUE, UINT64_MAX) != VK_SUCCESS) {
            throw std::runtime_error("Error calling renderer::Fence::await(): Failed to wait for frame completion");
        }
    }

    void Fence::reset() {
        if (vkResetFences(device_->getVkDevice(), 1, &fence_) != VK_SUCCESS) {
            throw std::runtime_error("Error calling renderer::Fence::reset(): Failed to reset fence");
        }
    }

    VkFence& Fence::getVkFence() {
        return fence_;
    }

    const VkFence& Fence::getVkFence() const {
        return fence_;
    }
}