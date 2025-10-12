#pragma once

#include <renderer/queue.hpp>

#include <limits>
#include <span>

#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

namespace renderer {
    class Instance;
    class Surface;
    class Pipeline;

    struct PipelineCreateInfo;

    // @brief Creation information for a device
    struct DeviceCreateInfo {
        Instance& instance;

        std::vector<QueueInfo> queues;
    };

    class Device {
    public:
        static Device create(const DeviceCreateInfo& createInfo);
        static void destroy(Device& device);

        static bool waitIdle(Device& device);

        static bool waitForFences(Device& device, const std::vector<Fence>& fences, bool waitAll = true, std::uint32_t timeout = std::numeric_limits<std::uint32_t>::max());
        static bool resetFences(Device& device, const std::vector<Fence>& fences);

        static std::vector<Pipeline> createPipelines(Device& device, const std::vector<PipelineCreateInfo>& createInfos);

        static std::span<Queue> getQueues(Device& device);

    private:
        VkDevice device_ = nullptr;
        VmaAllocator allocator_ = nullptr;
        Instance* instance_ = nullptr;

        std::vector<Queue> queues_;

        friend class CommandPool;
        friend class Buffer;
        friend class ShaderModule;
        friend class Semaphore;
        friend class Fence;
        friend class Sampler;
        friend class Framebuffer;
        friend class DescriptorSetLayout;
        friend class DescriptorPool;
        friend class PipelineLayout;
        friend class Pipeline;
        friend class ImageView;
        friend class Image;
        friend class RenderPass;
        friend class Swapchain;
    };
}