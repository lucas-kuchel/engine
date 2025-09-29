#pragma once

#include <data/references.hpp>

#include <vulkan/vulkan.h>

namespace renderer {
    class Device;

    // @brief Creation information for a semaphore
    struct SemaphoreCreateInfo {
        Device& device;
    };

    // @brief Represents a synchronisation object GPU/GPU operations
    // @note Not safe to copy
    class Semaphore {
    public:
        Semaphore(const SemaphoreCreateInfo& createInfo);
        ~Semaphore();

        Semaphore(const Semaphore&) = delete;
        Semaphore(Semaphore&&) noexcept = default;

        Semaphore& operator=(const Semaphore&) = delete;
        Semaphore& operator=(Semaphore&&) noexcept = default;

        // @brief Provides the Vulkan VkSemaphore
        // @return The VkSemaphore
        [[nodiscard]] VkSemaphore& getVkSemaphore();

        // @brief Provides the Vulkan VkSemaphore
        // @return The VkSemaphore
        [[nodiscard]] const VkSemaphore& getVkSemaphore() const;

    private:
        VkSemaphore semaphore_;

        data::Reference<Device> device_;
    };

    // @brief Creation information for a fence
    struct FenceCreateInfo {
        Device& device;
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

        void await();
        void reset();

        // @brief Provides the Vulkan VkSemaphore
        // @return The VkSemaphore
        [[nodiscard]] VkFence& getVkFence();

        // @brief Provides the Vulkan VkSemaphore
        // @return The VkSemaphore
        [[nodiscard]] const VkFence& getVkFence() const;

    private:
        VkFence fence_;

        data::Reference<Device> device_;
    };
}