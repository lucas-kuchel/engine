#include <renderer/resources/fence.hpp>

#include <renderer/device.hpp>

namespace renderer {
    Fence::Fence(const FenceCreateInfo& createInfo)
        : device_(createInfo.device) {

        VkFenceCreateInfo fenceCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
            .pNext = nullptr,
            .flags = FenceCreateFlags::mapFrom(createInfo.createFlags),
        };

        if (vkCreateFence(device_->getVkDevice(), &fenceCreateInfo, nullptr, &fence_) != VK_SUCCESS) {
            throw std::runtime_error("Construction failed: renderer::Fence: Failed to create fence");
        }
    }

    Fence::~Fence() {
        if (fence_ != VK_NULL_HANDLE) {
            vkDestroyFence(device_->getVkDevice(), fence_, nullptr);
        }
    }

    VkFence& Fence::getVkFence() {
        return fence_;
    }

    const VkFence& Fence::getVkFence() const {
        return fence_;
    }
}