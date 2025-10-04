#pragma once

#include <renderer/resources/config.hpp>

#include <data/references.hpp>

#include <vulkan/vulkan.h>

namespace renderer {
    class Device;

    // @brief Creation information for a fence
    struct FenceCreateInfo {
        Device& device;
        Flags createFlags;
    };

    // @brief Represents a synchronisation object GPU/CPU operations
    // @note Not safe to copy
    class Fence {
    public:
        Fence(const FenceCreateInfo& createInfo);
        ~Fence();

        Fence(const Fence&) = delete;
        Fence(Fence&&) noexcept = default;

        Fence& operator=(const Fence&) = delete;
        Fence& operator=(Fence&&) noexcept = default;

        // @brief Provides the Vulkan VkSemaphore
        // @return The VkSemaphore
        [[nodiscard]] VkFence& getVkFence();

        // @brief Provides the Vulkan VkSemaphore
        // @return The VkSemaphore
        [[nodiscard]] const VkFence& getVkFence() const;

    private:
        VkFence fence_ = VK_NULL_HANDLE;

        data::Reference<Device> device_;
    };
}