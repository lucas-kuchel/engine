#pragma once

#include <renderer/configuration.hpp>

#include <vulkan/vulkan.h>

namespace renderer {
    class Device;

    // @brief Creation information for a fence
    struct FenceCreateInfo {
        Device& device;
        Flags createFlags;
    };

    class Fence {
    public:
        static Fence create(const FenceCreateInfo& createInfo);
        static void destroy(Fence& fence);

    private:
        VkFence fence_ = nullptr;
        Device* device_ = nullptr;

        friend class Device;
        friend class Queue;
    };
}