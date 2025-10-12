#include <renderer/device.hpp>
#include <renderer/fence.hpp>

namespace renderer {
    Fence Fence::create(const FenceCreateInfo& createInfo) {
        Fence fence;

        VkFenceCreateInfo fenceCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
            .pNext = nullptr,
            .flags = FenceCreateFlags::mapFrom(createInfo.createFlags),
        };

        if (vkCreateFence(createInfo.device.device_, &fenceCreateInfo, nullptr, &fence.fence_) != VK_SUCCESS) {
            fence.fence_ = nullptr;
        }
        else {
            fence.device_ = &createInfo.device;
        }

        return fence;
    }

    void Fence::destroy(Fence& fence) {
        if (fence.fence_ != VK_NULL_HANDLE) {
            vkDestroyFence(fence.device_->device_, fence.fence_, nullptr);
        }
    }
}