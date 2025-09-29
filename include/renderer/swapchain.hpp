#pragma once

#include <data/references.hpp>

#include <renderer/resources/image.hpp>
#include <renderer/resources/pass.hpp>

#include <renderer/commands/buffer.hpp>
#include <renderer/commands/sync.hpp>

#include <vulkan/vulkan.h>

#include <span>

namespace renderer {
    class Instance;
    class Surface;
    class Device;
    class Queue;

    // @brief Creation information for a swapchain
    struct SwapchainCreateInfo {
        Instance& instance;
        Surface& surface;
        Device& device;
        Queue& presentQueue;

        // @brief How many images you want the swapchain to posess
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
        // @note Blocks while the provided command sync is not complete
        // @note Will attempt to acquire up to 10 times if resizing is required
        // @throws std::runtime_error if the swapchain is invalid or if the recreation limit is exceeeded
        void acquireNextImage(Semaphore& available);

        // @brief Presents the selected next image
        // @param The semaphore for indicating GPU operation completion
        // @note Blocks while the provided command sync is not complete
        // @throws std::runtime_error if the swapchain is invalid or if presentation fails
        void presentNextImage(Semaphore& finished);

        // @brief Sets if synchronisation should be enabled
        // @param If the swapchain should synchronise
        void setSynchronisation(bool sync);

        // @brief Sets the image count
        // @param Requested image count
        // @note This may be disregarded as the driver has the final say
        void setImageCount(std::uint32_t count);

        // @brief Provides the backend-agnostic image format
        // @return The image format of all images in the Swapchain
        [[nodiscard]] ImageFormat getImageFormat() const;

        // @brief Provides the number of frames in the swapchain
        // @return Frame count of the swapchain
        [[nodiscard]] std::uint32_t getFrameCount() const;

        // @brief Provides the current frame index of the swapchain
        // @return Current frame index of the swapchain
        [[nodiscard]] std::uint32_t getFrameIndex() const;

        // @brief Provides all image views of the swapchain
        // @return Image views of the swapchain
        [[nodiscard]] std::span<ImageView> getImageViews();

        // @brief Returns if synchronisation is enabled (VSync)
        // @return Whether synchronisation is enabled
        [[nodiscard]] bool isSynchronised() const;

        // @brief Returns if the swapchain is flagged for recreation
        // @note The swapchain will be recreated automatically at the next call of acquireNextImage
        // @return Whether the swapchain will be recreated
        [[nodiscard]] bool needsRecreate() const;

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
        VkSwapchainKHR swapchain_;
        VkPresentModeKHR presentMode_;
        VkSurfaceFormatKHR surfaceFormat_;
        VkExtent2D extent_;

        data::NullableReference<Instance> instance_;
        data::NullableReference<Surface> surface_;
        data::NullableReference<Device> device_;
        data::NullableReference<Queue> presentQueue_;

        std::uint32_t imageCount_;
        std::uint32_t imageIndex_ = 0;

        std::vector<Image> images_;
        std::vector<ImageView> imageViews_;

        bool synchronise_;
        bool recreateSwapchain_ = false;

        VkSurfaceCapabilitiesKHR getSurfaceCapabilities();
        void createImageResources(const VkSwapchainCreateInfoKHR& swapchainCreateInfo);

        void selectSurfaceFormat();
        void selectPresentMode();
        void recreateSwapchain();
    };
}