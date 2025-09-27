#pragma once

#include <data/references.hpp>

#include <renderer/resources/image.hpp>

#include <vulkan/vulkan.h>

namespace renderer {
    class Instance;
    class Surface;
    class Device;

    struct SwapchainData {
        VkSwapchainKHR swapchain = VK_NULL_HANDLE;
        Device* device = nullptr;

        std::size_t imageCount = 3;

        std::vector<ImageHandle> images;
        std::vector<ImageViewHandle> imageViews;

        bool synchronise = true;
    };

    struct SwapchainCreateInfo {
        Instance& instance;
        Surface& surface;
        Device& device;

        std::size_t imageCount;

        bool synchronise;
    };

    class Swapchain {
    public:
        Swapchain();
        ~Swapchain();

        Swapchain(const Swapchain&) = delete;
        Swapchain(Swapchain&&) noexcept = default;

        Swapchain& operator=(const Swapchain&) = delete;
        Swapchain& operator=(Swapchain&&) noexcept = default;

        void create(const SwapchainCreateInfo& createInfo);

        SwapchainData& getData();

    private:
        SwapchainData data_;
    };
}