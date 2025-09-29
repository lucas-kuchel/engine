#pragma once

#include <data/references.hpp>
#include <data/registry.hpp>

#include <renderer/queue.hpp>

#include <renderer/resources/framebuffer.hpp>
#include <renderer/resources/image.hpp>
#include <renderer/resources/pass.hpp>

#include <span>

#include <vulkan/vulkan.h>

namespace renderer {
    class Instance;
    class Surface;

    // @brief Creation information for a device
    struct DeviceCreateInfo {
        Instance& instance;

        std::vector<QueueCreateInfo> queues;
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

        // @brief Queries the queues from the device
        // @return List of all usable queues
        [[nodiscard]] std::span<Queue> getQueues();

        // @brief Queries the queues from the device
        // @return List of all usable queues
        [[nodiscard]] std::span<const Queue> getQueues() const;

        // @brief Provides the Vulkan VkDevice
        // @return The VkDevice
        [[nodiscard]] VkDevice& getVkDevice();

        // @brief Provides the Vulkan VkDevice
        // @return The VkDevice
        [[nodiscard]] const VkDevice& getVkDevice() const;

    private:
        VkDevice device_ = VK_NULL_HANDLE;

        data::Reference<Instance> instance_;

        std::vector<Queue> queues_;
    };
}