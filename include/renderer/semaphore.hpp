#pragma once

#include <vulkan/vulkan.h>

namespace renderer {
    class Device;

    class Semaphore {
    public:
        static Semaphore create(Device& device);
        static void destroy(Semaphore& semaphore);

    private:
        VkSemaphore semaphore_ = nullptr;
        Device* device_ = nullptr;

        friend class Queue;
        friend class Swapchain;
    };
}