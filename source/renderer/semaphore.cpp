#include <renderer/device.hpp>
#include <renderer/semaphore.hpp>

namespace renderer {
    Semaphore Semaphore::create(Device& device) {
        Semaphore semaphore;

        VkSemaphoreCreateInfo semaphoreCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
        };

        if (vkCreateSemaphore(device.device_, &semaphoreCreateInfo, nullptr, &semaphore.semaphore_) != VK_SUCCESS) {
            semaphore.semaphore_ = nullptr;
        }
        else {
            semaphore.device_ = &device;
        }

        return semaphore;
    }

    void Semaphore::destroy(Semaphore& semaphore) {
        if (semaphore.semaphore_ != VK_NULL_HANDLE) {
            vkDestroySemaphore(semaphore.device_->device_, semaphore.semaphore_, nullptr);
        }
    }
}