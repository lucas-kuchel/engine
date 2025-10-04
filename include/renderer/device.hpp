#pragma once

#include <data/references.hpp>
#include <data/registry.hpp>

#include <renderer/queue.hpp>

#include <renderer/resources/fence.hpp>
#include <renderer/resources/framebuffer.hpp>
#include <renderer/resources/image.hpp>
#include <renderer/resources/pass.hpp>
#include <renderer/resources/pipeline.hpp>

#include <span>

#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

namespace renderer {
    class Instance;
    class Surface;

    // @brief Creation information for a device
    struct DeviceCreateInfo {
        Instance& instance;

        std::vector<QueueInfo> queues;
    };

    // @brief Represents the device of the renderer
    // @note Not safe to copy
    class Device {
    public:
        Device(const DeviceCreateInfo& createInfo);
        ~Device();

        Device(const Device&) = delete;
        Device(Device&&) noexcept = default;

        Device& operator=(const Device&) = delete;
        Device& operator=(Device&&) noexcept = default;

        // @brief Blocks until the GPU is not busy
        void waitIdle();

        // @brief Blocks until fences have been signalled
        // @param List of fences to wait for
        // @param If all fences should be waited for. Defaults to true
        // @param Timeout period. Defaults to UINT32_MAX
        void waitForFences(const data::ReferenceList<Fence>& fences, bool waitAll = true, std::uint32_t timeout = std::numeric_limits<std::uint32_t>::max());

        // @brief Resets all fences to be unsignalled
        // @param List of fences to reset
        // @note Must be called when all fences are not pending
        void resetFences(const data::ReferenceList<Fence>& fences);

        // @brief Creates pipelines based on provided infos
        // @note In the future, pipeline cache systems will be added to this
        // @param List of pipeline creation infos
        // @return List of created pipelines
        // @note Pipelines are returned in the order the create infos are provided
        [[nodiscard]] std::vector<Pipeline> createPipelines(const std::vector<PipelineCreateInfo>& createInfos);

        // @brief Queries the queues from the device
        // @return List of all usable queues
        [[nodiscard]] std::span<Queue> queues();

        [[nodiscard]] Instance& getInstance();
        [[nodiscard]] const Instance& getInstance() const;

        // @brief Queries the queues from the device
        // @return List of all usable queues
        [[nodiscard]] std::span<const Queue> queues() const;

        // @brief Provides the Vulkan VkDevice
        // @return The VkDevice
        [[nodiscard]] VkDevice& getVkDevice();

        // @brief Provides the Vulkan VkDevice
        // @return The VkDevice
        [[nodiscard]] const VkDevice& getVkDevice() const;

        // @brief Provides the Vulkan Memory Allocator VmaAllocator
        // @return The VmaAllocator
        [[nodiscard]] VmaAllocator& getVmaAllocator();

        // @brief Provides the Vulkan Memory Allocator VmaAllocator
        // @return The VmaAllocator
        [[nodiscard]] const VmaAllocator& getVmaAllocator() const;

    private:
        VkDevice device_ = VK_NULL_HANDLE;
        VmaAllocator allocator_ = VK_NULL_HANDLE;

        data::Ref<Instance> instance_;

        std::vector<Queue> queues_;
    };
}