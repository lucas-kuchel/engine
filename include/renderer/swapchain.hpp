#pragma once

#include <renderer/configuration.hpp>

#include <cstdint>
#include <span>
#include <vector>

#include <glm/glm.hpp>

#include <vulkan/vulkan.h>

namespace renderer {
    class Instance;
    class Surface;
    class Device;
    class Queue;
    class Semaphore;
    class ImageView;
    class Image;
    class Swapchain;

    // @brief Creation information for a swapchain
    struct SwapchainCreateInfo {
        Surface& surface;
        Device& device;

        // @brief Ref to a queue that supports presentation operations
        Queue& presentQueue;

        // @brief How many images you want the swapchain to posess
        // @note This may be disregarded as the driver has the final say
        std::uint32_t imageCount;

        // @brief Whether presentation should be synchronised with vertical refresh rate (VSync)
        // @note Vulkan will always prefer VK_PRESENT_MODE_MAILBOX_KHR when available
        bool synchronise;

        Swapchain* oldSwapchain = nullptr;
    };

    class Swapchain {
    public:
        static Swapchain create(const SwapchainCreateInfo& createInfo);
        static void destroy(Swapchain& swapchain);

        static bool acquireNextImage(Swapchain& swapchain, Semaphore& acquireSemaphore);
        static bool presentNextImage(Swapchain& swapchain, Semaphore& presentSemaphore);

        static ImageFormat getFormat(Swapchain& swapchain);
        static std::uint32_t getImageCount(Swapchain& swapchain);
        static std::uint32_t getImageIndex(Swapchain& swapchain);
        static std::span<const Image> getImages(Swapchain& swapchain);
        static std::span<const ImageView> getImageViews(Swapchain& swapchain);
        static bool isSynchronised(Swapchain& swapchain);
        static bool shouldRecreate(Swapchain& swapchain);
        static glm::uvec2 getExtent(Swapchain& swapchain);

    private:
        VkSwapchainKHR swapchain_ = nullptr;
        Instance* instance_ = nullptr;
        Surface* surface_ = nullptr;
        Device* device_ = nullptr;
        Queue* presentQueue_ = nullptr;

        VkPresentModeKHR presentMode_;
        VkSurfaceFormatKHR surfaceFormat_;
        VkExtent2D extent_;

        std::uint32_t imageCount_ = 0;
        std::uint32_t imageIndex_ = 0;

        std::vector<Image> images_;
        std::vector<ImageView> imageViews_;

        bool synchronise_ = false;
        bool recreate_ = false;

        static VkSurfaceCapabilitiesKHR getSurfaceCapabilities(Swapchain& swapchain);

        static void createImageResources(Swapchain& swapchain);
        static void selectSurfaceFormat(Swapchain& swapchain);
        static void selectPresentMode(Swapchain& swapchain);
    };
}