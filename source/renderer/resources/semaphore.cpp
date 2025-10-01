#include <renderer/resources/semaphore.hpp>

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
            throw std::runtime_error("Construction failed: renderer::Semaphore: Failed to create semaphore");
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
}