#include <renderer/device.hpp>
#include <renderer/instance.hpp>
#include <renderer/queue.hpp>
#include <renderer/surface.hpp>
#include <renderer/swapchain.hpp>

#include <app/window.hpp>

#include <algorithm>
#include <limits>
#include <stdexcept>

namespace renderer {
    Swapchain::Swapchain(const SwapchainCreateInfo& createInfo)
        : instance_(createInfo.device.getInstance()), surface_(createInfo.surface),
          device_(createInfo.device), presentQueue_(createInfo.presentQueue),
          imageCount_(createInfo.imageCount), synchronise_(createInfo.synchronise) {
        recreateSwapchain();
    }

    Swapchain::~Swapchain() {
        if (swapchain_ != VK_NULL_HANDLE) {
            vkDestroySwapchainKHR(device_->getVkDevice(), swapchain_, nullptr);
        }

        for (auto& image : images_) {
            image.image_ = VK_NULL_HANDLE;
        }
    }

    std::uint32_t Swapchain::acquireNextImage(Semaphore& available) {
        if (recreate_) {
            throw std::runtime_error("Call failed: renderer::Swapchain::acquireNextImage(): Swapchain requires resizing; cannot acquire next image");
        }

        VkResult result = vkAcquireNextImageKHR(device_->getVkDevice(), swapchain_, UINT32_MAX, available.getVkSemaphore(), VK_NULL_HANDLE, &imageIndex_);

        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            recreate_ = true;
        }
        else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            throw std::runtime_error("Call failed: renderer::Swapchain::acquireNextImage(): Failed to acquire next image for presentation");
        }

        return imageIndex_;
    }

    void Swapchain::recreate(const SwapchainRecreateInfo& recreateInfo) {
        synchronise_ = recreateInfo.synchronise;
        imageCount_ = recreateInfo.imageCount;

        recreateSwapchain();

        recreate_ = false;
    }

    void Swapchain::presentNextImage(Semaphore& finished) {
        auto& queue = presentQueue_->getVkQueue();

        VkPresentInfoKHR presentInfo = {
            .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            .pNext = nullptr,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &finished.getVkSemaphore(),
            .swapchainCount = 1,
            .pSwapchains = &swapchain_,
            .pImageIndices = &imageIndex_,
            .pResults = nullptr,
        };

        VkResult result = vkQueuePresentKHR(queue, &presentInfo);

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
            recreate_ = true;
        }
        else if (result != VK_SUCCESS) {
            throw std::runtime_error("Call failed: renderer::Swapchain::presentNextImage(): Failed to present image");
        }
    }

    ImageFormat Swapchain::format() const {
        return Image::reverseMapFormat(surfaceFormat_.format);
    }

    std::uint32_t Swapchain::imageCount() const {
        return imageCount_;
    }

    std::uint32_t Swapchain::getImageIndex() const {
        return imageIndex_;
    }

    std::span<ImageView> Swapchain::images() {
        return imageViews_;
    }

    bool Swapchain::synchronised() const {
        return synchronise_;
    }

    bool Swapchain::shouldRecreate() const {
        return recreate_;
    }

    data::Extent2D<std::uint32_t> Swapchain::extent() const {
        return {extent_.width, extent_.height};
    }

    VkSwapchainKHR& Swapchain::getVkSwapchainKHR() {
        return swapchain_;
    }

    VkPresentModeKHR& Swapchain::getVkPresentModeKHR() {
        return presentMode_;
    }

    VkSurfaceFormatKHR& Swapchain::getVkSurfaceFormatKHR() {
        return surfaceFormat_;
    }

    const VkSwapchainKHR& Swapchain::getVkSwapchainKHR() const {
        return swapchain_;
    }

    const VkPresentModeKHR& Swapchain::getVkPresentModeKHR() const {
        return presentMode_;
    }

    const VkSurfaceFormatKHR& Swapchain::getVkSurfaceFormatKHR() const {
        return surfaceFormat_;
    }

    void Swapchain::createImageResources() {
        auto& device = device_->getVkDevice();

        std::uint32_t actualImageCount = 0;

        if (vkGetSwapchainImagesKHR(device, swapchain_, &actualImageCount, nullptr) != VK_SUCCESS) {
            throw std::runtime_error("Construction failed: renderer::Swapchain: Failed to query swapchain images");
        }

        std::vector<VkImage> queriedImages(actualImageCount);

        if (vkGetSwapchainImagesKHR(device, swapchain_, &actualImageCount, queriedImages.data()) != VK_SUCCESS) {
            throw std::runtime_error("Construction failed: renderer::Swapchain: Failed to query swapchain images");
        }

        images_.reserve(actualImageCount);
        imageViews_.reserve(actualImageCount);

        for (std::size_t i = 0; i < actualImageCount; i++) {
            images_.push_back(Image(device_.get()));

            Image& image = images_.back();

            image.image_ = queriedImages[i];
            image.extent_ = {extent_.width, extent_.height, 1};
            image.format_ = surfaceFormat_.format;
            image.type_ = VK_IMAGE_TYPE_2D;
            image.arrayLayers_ = 1;
            image.mipLevels_ = 1;
            image.sampleCount_ = 1;

            ImageViewCreateInfo viewCreateInfo = {
                .image = image,
                .type = ImageViewType::IMAGE_2D,
                .aspectFlags = ImageAspectFlags::COLOUR,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1,
            };

            imageViews_.emplace_back(viewCreateInfo);
        }
    }

    VkSurfaceCapabilitiesKHR Swapchain::getSurfaceCapabilities() {
        auto& physicalDevice = instance_->getVkPhysicalDevice();
        auto& surface = surface_->getVkSurface();
        auto extent = surface_->extent();

        VkSurfaceCapabilitiesKHR surfaceCapabilities;

        if (vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &surfaceCapabilities) != VK_SUCCESS) {
            throw std::runtime_error("Call failed: renderer::Swapchain::create(): Failed to enumerate surface capabilities");
        }

        if (surfaceCapabilities.currentExtent.width != std::numeric_limits<std::uint32_t>::max()) {
            extent_ = surfaceCapabilities.currentExtent;
        }
        else {
            auto& minExtent = surfaceCapabilities.minImageExtent;
            auto& maxExtent = surfaceCapabilities.maxImageExtent;

            extent_.width = std::clamp(extent.width, minExtent.width, maxExtent.width);
            extent_.height = std::clamp(extent.height, minExtent.height, maxExtent.height);
        }

        return surfaceCapabilities;
    }

    void Swapchain::selectSurfaceFormat() {
        auto& physicalDevice = instance_->getVkPhysicalDevice();
        auto& surface = surface_->getVkSurface();

        std::uint32_t formatCount = 0;

        if (vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr) != VK_SUCCESS) {
            throw std::runtime_error("Call failed: renderer::Swapchain::create(): Failed to get surface formats");
        }

        std::vector<VkSurfaceFormatKHR> formats(formatCount);

        if (vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, formats.data()) != VK_SUCCESS) {
            throw std::runtime_error("Call failed: renderer::Swapchain::create(): Failed to get surface formats");
        }

        surfaceFormat_ = formats[0];

        if (formats.size() == 1 && formats[0].format == VK_FORMAT_UNDEFINED) {
            surfaceFormat_.format = VK_FORMAT_B8G8R8A8_SRGB;
            surfaceFormat_.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
        }
        else {
            for (const auto& format : formats) {
                if (format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                    surfaceFormat_ = format;
                    break;
                }
            }
        }
    }

    void Swapchain::selectPresentMode() {
        auto& physicalDevice = instance_->getVkPhysicalDevice();
        auto& surface = surface_->getVkSurface();

        std::uint32_t presentModeCount = 0;

        if (vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr) != VK_SUCCESS) {
            throw std::runtime_error("Call failed: renderer::Swapchain::create(): Failed to get surface present modes");
        }

        std::vector<VkPresentModeKHR> presentModes(presentModeCount);

        if (vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, presentModes.data()) != VK_SUCCESS) {
            throw std::runtime_error("Call failed: renderer::Swapchain::create(): Failed to get surface present modes");
        }

        presentMode_ = VK_PRESENT_MODE_FIFO_KHR;

        std::int32_t chosenPriority = -1;

        for (const auto& mode : presentModes) {
            std::int32_t priority = -1;

            if (mode == VK_PRESENT_MODE_MAILBOX_KHR) {
                priority = 3;
            }
            else if (!synchronise_ && mode == VK_PRESENT_MODE_IMMEDIATE_KHR) {
                priority = 2;
            }
            else if (mode == VK_PRESENT_MODE_FIFO_RELAXED_KHR) {
                priority = 1;
            }
            else if (mode == VK_PRESENT_MODE_FIFO_KHR) {
                priority = 0;
            }

            if (priority > chosenPriority) {
                chosenPriority = priority;
                presentMode_ = mode;
            }
        }
    }

    void Swapchain::recreateSwapchain() {
        auto& device = device_->getVkDevice();
        auto& surface = surface_->getVkSurface();

        VkSwapchainKHR oldSwapchain = swapchain_;

        swapchain_ = VK_NULL_HANDLE;

        for (auto& image : images_) {
            image.image_ = VK_NULL_HANDLE;
        }

        images_.clear();
        imageViews_.clear();

        VkSurfaceCapabilitiesKHR surfaceCapabilities = getSurfaceCapabilities();

        selectSurfaceFormat();
        selectPresentMode();

        imageCount_ = std::max(std::min(imageCount_, surfaceCapabilities.maxImageCount), surfaceCapabilities.minImageCount);

        VkSwapchainCreateInfoKHR swapchainCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
            .pNext = nullptr,
            .flags = 0,
            .surface = surface,
            .minImageCount = imageCount_,
            .imageFormat = surfaceFormat_.format,
            .imageColorSpace = surfaceFormat_.colorSpace,
            .imageExtent = extent_,
            .imageArrayLayers = 1,
            .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
            .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .queueFamilyIndexCount = 0,
            .pQueueFamilyIndices = nullptr,
            .preTransform = surfaceCapabilities.currentTransform,
            .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
            .presentMode = presentMode_,
            .clipped = VK_TRUE,
            .oldSwapchain = oldSwapchain,
        };

        if (vkCreateSwapchainKHR(device, &swapchainCreateInfo, nullptr, &swapchain_) != VK_SUCCESS) {
            throw std::runtime_error("Construction failed: renderer::Swapchain: Failed to create swapchain");
        }

        if (oldSwapchain != VK_NULL_HANDLE) {
            vkDestroySwapchainKHR(device, oldSwapchain, nullptr);
        }

        createImageResources();
    }
}