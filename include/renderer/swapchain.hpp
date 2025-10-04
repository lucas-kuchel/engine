#pragma once

#include <data/references.hpp>

#include <renderer/resources/fence.hpp>
#include <renderer/resources/image.hpp>
#include <renderer/resources/pass.hpp>
#include <renderer/resources/semaphore.hpp>

#include <renderer/commands/buffer.hpp>

#include <vulkan/vulkan.h>

#include <span>

namespace renderer {
    class Instance;
    class Surface;
    class Device;
    class Queue;

    // @brief Creation information for a swapchain
    struct SwapchainCreateInfo {
        Surface& surface;
        Device& device;

        // @brief Reference to a queue that supports presentation operations
        Queue& presentQueue;

        // @brief How many images you want the swapchain to posess
        // @note This may be disregarded as the driver has the final say
        std::uint32_t imageCount;

        // @brief Whether presentation should be synchronised with vertical refresh rate (VSync)
        // @note Vulkan will always prefer VK_PRESENT_MODE_MAILBOX_KHR when available
        bool synchronise;
    };

    // @brief Recreation information for a swapchain
    struct SwapchainRecreateInfo {
        // @brief How many images you want the swapchain to possess
        // @note This may be disregarded as the driver has the final say
        std::uint32_t imageCount;

        // @brief Whether presentation should be synchronised with vertical refresh rate (VSync)
        // @note Vulkan will always prefer VK_PRESENT_MODE_MAILBOX_KHR when available
        bool synchronise;
    };

    // @brief Manages images shown to window
    // @not Not safe to copy
    class Swapchain {
    public:
        Swapchain(const SwapchainCreateInfo& createInfo);
        ~Swapchain();

        Swapchain(const Swapchain&) = delete;
        Swapchain(Swapchain&&) noexcept = default;

        Swapchain& operator=(const Swapchain&) = delete;
        Swapchain& operator=(Swapchain&&) noexcept = default;

        // @brief Selects the next image to render to
        // @param The semaphore to signal GPU availability
        // @return The next image index
        // @throws std::runtime_error if the swapchain is invalid
        [[nodiscard]] std::uint32_t acquireNextImage(Semaphore& available);

        // @brief Recreates the swapchain with the provided information
        // @param The recreation information
        void recreate(const SwapchainRecreateInfo& recreateInfo);

        // @brief Presents the selected next image
        // @param The semaphore for indicating GPU operation completion
        // @note Blocks while the provided command sync is not complete
        // @throws std::runtime_error if the swapchain is invalid or if presentation fails
        void presentNextImage(Semaphore& finished);

        // @brief Provides the backend-agnostic image format
        // @return The image format of all images in the Swapchain
        [[nodiscard]] ImageFormat getImageFormat() const;

        // @brief Provides the number of images in the swapchain
        // @return Frame count of the swapchain
        [[nodiscard]] std::uint32_t getImageCount() const;

        // @brief Provides the current image index of the swapchain
        // @return Current frame index of the swapchain
        [[nodiscard]] std::uint32_t getImageIndex() const;

        // @brief Provides all image views of the swapchain
        // @return Image views of the swapchain
        [[nodiscard]] std::span<ImageView> getImageViews();

        // @brief Returns if synchronisation is enabled (VSync)
        // @return Whether synchronisation is enabled
        [[nodiscard]] bool isSynchronised() const;

        // @brief Returns if the swapchain is flagged for recreation
        // @note This flag must be acted upon if true
        // @return Whether the swapchain will be recreated
        [[nodiscard]] bool shouldRecreate() const;

        // @brief Returns the current extent of the swapchain
        // @return The current extent of the swapchain
        [[nodiscard]] data::Extent2D<std::uint32_t> getExtent() const;

        // @brief Provides the Vulkan swapchain handle
        // @return the VkSwapchainKHR handle
        [[nodiscard]] VkSwapchainKHR& getVkSwapchainKHR();

        // @brief Provides the selected Vulkan presentation mode
        // @return the VkPresentModeKHR enum
        [[nodiscard]] VkPresentModeKHR& getVkPresentModeKHR();

        // @brief Provides the selected Vulkan surface format
        // @return the VkSurfaceFormatKHR handle
        [[nodiscard]] VkSurfaceFormatKHR& getVkSurfaceFormatKHR();

        // @brief Provides the Vulkan swapchain handle
        // @return the VkSwapchainKHR handle
        [[nodiscard]] const VkSwapchainKHR& getVkSwapchainKHR() const;

        // @brief Provides the selected Vulkan presentation mode
        // @return the VkPresentModeKHR enum
        [[nodiscard]] const VkPresentModeKHR& getVkPresentModeKHR() const;

        // @brief Provides the selected Vulkan surface format
        // @return the VkSurfaceFormatKHR handle
        [[nodiscard]] const VkSurfaceFormatKHR& getVkSurfaceFormatKHR() const;

    private:
        VkSwapchainKHR swapchain_ = VK_NULL_HANDLE;
        VkPresentModeKHR presentMode_;
        VkSurfaceFormatKHR surfaceFormat_;
        VkExtent2D extent_;

        data::Reference<Instance> instance_;
        data::Reference<Surface> surface_;
        data::Reference<Device> device_;
        data::Reference<Queue> presentQueue_;

        std::uint32_t imageCount_;
        std::uint32_t imageIndex_ = 0;

        std::vector<Image> images_;
        std::vector<ImageView> imageViews_;

        bool synchronise_;
        bool recreate_ = false;

        VkSurfaceCapabilitiesKHR getSurfaceCapabilities();

        void createImageResources();
        void selectSurfaceFormat();
        void selectPresentMode();
        void recreateSwapchain();
    };
}