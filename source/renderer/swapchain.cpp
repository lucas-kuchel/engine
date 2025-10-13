#include <renderer/device.hpp>
#include <renderer/image.hpp>
#include <renderer/image_view.hpp>
#include <renderer/instance.hpp>
#include <renderer/queue.hpp>
#include <renderer/semaphore.hpp>
#include <renderer/surface.hpp>
#include <renderer/swapchain.hpp>

#include <app/window.hpp>

#include <algorithm>
#include <limits>
#include <stdexcept>

namespace renderer {
    std::pair<Swapchain, SwapchainCreateResult> Swapchain::create(const SwapchainCreateInfo& createInfo) {
        Swapchain swapchain;

        VkSwapchainKHR oldSwapchain = nullptr;

        swapchain.synchronise_ = createInfo.synchronise;
        swapchain.instance_ = createInfo.device.instance_;
        swapchain.device_ = &createInfo.device;
        swapchain.surface_ = &createInfo.surface;
        swapchain.presentQueue_ = &createInfo.presentQueue;
        swapchain.recreate_ = false;

        VkSurfaceCapabilitiesKHR surfaceCapabilities = getSurfaceCapabilities(swapchain);

        if (surfaceCapabilities.currentExtent.width == 0 || surfaceCapabilities.currentExtent.height == 0) {
            return {Swapchain(), SwapchainCreateResult::PENDING};
        }

        selectSurfaceFormat(swapchain);
        selectPresentMode(swapchain);

        std::uint32_t imageCountMaximum = std::min(createInfo.imageCount, surfaceCapabilities.maxImageCount);
        swapchain.imageCount_ = std::max(imageCountMaximum, surfaceCapabilities.minImageCount);

        if (surfaceCapabilities.currentExtent.width != std::numeric_limits<std::uint32_t>::max()) {
            swapchain.extent_ = surfaceCapabilities.currentExtent;
        }

        swapchain.extent_.width = std::clamp(swapchain.extent_.width, surfaceCapabilities.minImageExtent.width, surfaceCapabilities.maxImageExtent.width);
        swapchain.extent_.height = std::clamp(swapchain.extent_.height, surfaceCapabilities.minImageExtent.height, surfaceCapabilities.maxImageExtent.height);

        VkSwapchainCreateInfoKHR swapchainCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
            .pNext = nullptr,
            .flags = 0,
            .surface = createInfo.surface.surface_,
            .minImageCount = createInfo.imageCount,
            .imageFormat = swapchain.surfaceFormat_.format,
            .imageColorSpace = swapchain.surfaceFormat_.colorSpace,
            .imageExtent = swapchain.extent_,
            .imageArrayLayers = 1,
            .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
            .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .queueFamilyIndexCount = 0,
            .pQueueFamilyIndices = nullptr,
            .preTransform = surfaceCapabilities.currentTransform,
            .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
            .presentMode = swapchain.presentMode_,
            .clipped = VK_TRUE,
            .oldSwapchain = oldSwapchain,
        };

        SwapchainCreateResult createResult = SwapchainCreateResult::SUCCESS;

        VkResult error = vkCreateSwapchainKHR(createInfo.device.device_, &swapchainCreateInfo, nullptr, &swapchain.swapchain_);

        if (error == VK_ERROR_NATIVE_WINDOW_IN_USE_KHR) {
            if (createInfo.oldSwapchain) {
                Swapchain::destroy(*createInfo.oldSwapchain);
            }

            swapchainCreateInfo.oldSwapchain = nullptr;

            error = vkCreateSwapchainKHR(createInfo.device.device_, &swapchainCreateInfo, nullptr, &swapchain.swapchain_);
        }

        if (error == VK_ERROR_NATIVE_WINDOW_IN_USE_KHR) {
            createResult = SwapchainCreateResult::FAILED;
        }
        else if (error == VK_ERROR_OUT_OF_DATE_KHR) {
            createResult = SwapchainCreateResult::PENDING;
        }
        else if (error != VK_SUCCESS) {
            createResult = SwapchainCreateResult::FAILED;
        }
        else {
            if (createInfo.oldSwapchain) {
                Swapchain::destroy(*createInfo.oldSwapchain);
            }

            createImageResources(swapchain);
        }

        return {swapchain, createResult};
    }

    void Swapchain::destroy(Swapchain& swapchain) {
        if (swapchain.swapchain_) {
            for (auto& imageView : swapchain.imageViews_) {
                ImageView::destroy(imageView);
            }

            swapchain.images_.clear();
            swapchain.imageViews_.clear();

            vkDestroySwapchainKHR(swapchain.device_->device_, swapchain.swapchain_, nullptr);

            swapchain.swapchain_ = nullptr;
        }
    }

    bool Swapchain::acquireNextImage(Swapchain& swapchain, Semaphore& acquireSemaphore) {
        if (swapchain.recreate_) {
            return false;
        }

        VkResult result = vkAcquireNextImageKHR(swapchain.device_->device_, swapchain.swapchain_, UINT32_MAX, acquireSemaphore.semaphore_, VK_NULL_HANDLE, &swapchain.imageIndex_);

        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            swapchain.recreate_ = true;
        }
        else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            return false;
        }

        return true;
    }

    bool Swapchain::presentNextImage(Swapchain& swapchain, Semaphore& presentSemaphore) {
        auto& queue = swapchain.presentQueue_->queue_;

        VkPresentInfoKHR presentInfo = {
            .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            .pNext = nullptr,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &presentSemaphore.semaphore_,
            .swapchainCount = 1,
            .pSwapchains = &swapchain.swapchain_,
            .pImageIndices = &swapchain.imageIndex_,
            .pResults = nullptr,
        };

        VkResult result = vkQueuePresentKHR(queue, &presentInfo);

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
            swapchain.recreate_ = true;
        }
        else if (result != VK_SUCCESS) {
            return false;
        }

        return true;
    }

    ImageFormat Swapchain::getFormat(Swapchain& swapchain) {
        return Image::reverseMapFormat(swapchain.images_.front().format_);
    }

    std::uint32_t Swapchain::getImageCount(Swapchain& swapchain) {
        return swapchain.imageCount_;
    }

    std::uint32_t Swapchain::getImageIndex(Swapchain& swapchain) {
        return swapchain.imageIndex_;
    }

    std::span<const Image> Swapchain::getImages(Swapchain& swapchain) {
        return swapchain.images_;
    }

    std::span<const ImageView> Swapchain::getImageViews(Swapchain& swapchain) {
        return swapchain.imageViews_;
    }

    bool Swapchain::isSynchronised(Swapchain& swapchain) {
        return swapchain.synchronise_;
    }

    bool Swapchain::shouldRecreate(Swapchain& swapchain) {
        return swapchain.recreate_;
    }

    glm::uvec2 Swapchain::getExtent(Swapchain& swapchain) {
        return {swapchain.extent_.width, swapchain.extent_.height};
    }

    void Swapchain::createImageResources(Swapchain& swapchain) {
        auto& device = swapchain.device_->device_;

        std::uint32_t actualImageCount = 0;

        if (vkGetSwapchainImagesKHR(device, swapchain.swapchain_, &actualImageCount, nullptr) != VK_SUCCESS) {
            throw std::runtime_error("Construction failed: renderer::Swapchain: Failed to query swapchain images");
        }

        std::vector<VkImage> queriedImages(actualImageCount);

        if (vkGetSwapchainImagesKHR(device, swapchain.swapchain_, &actualImageCount, queriedImages.data()) != VK_SUCCESS) {
            throw std::runtime_error("Construction failed: renderer::Swapchain: Failed to query swapchain images");
        }

        swapchain.images_.reserve(actualImageCount);
        swapchain.imageViews_.reserve(actualImageCount);

        for (std::size_t i = 0; i < actualImageCount; i++) {
            swapchain.images_.push_back(Image());

            Image& image = swapchain.images_.back();

            image.isHostCoherent_ = false;
            image.isHostVisible_ = false;
            image.image_ = queriedImages[i];
            image.extent_ = {swapchain.extent_.width, swapchain.extent_.height, 1};
            image.format_ = swapchain.surfaceFormat_.format;
            image.type_ = VK_IMAGE_TYPE_2D;
            image.arrayLayers_ = 1;
            image.mipLevels_ = 1;
            image.sampleCount_ = 1;
            image.device_ = swapchain.device_;

            ImageViewCreateInfo viewCreateInfo = {
                .image = image,
                .type = ImageViewType::IMAGE_2D,
                .aspectFlags = ImageAspectFlags::COLOUR,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1,
            };

            swapchain.imageViews_.push_back(ImageView());

            auto& imageView = swapchain.imageViews_.back();

            imageView = ImageView::create(viewCreateInfo);
        }

        if (swapchain.imageCount_ != actualImageCount) {
            swapchain.imageCount_ = actualImageCount;
        }
    }

    VkSurfaceCapabilitiesKHR Swapchain::getSurfaceCapabilities(Swapchain& swapchain) {
        auto& physicalDevice = swapchain.instance_->physicalDevice_;
        auto& surface = swapchain.surface_->surface_;
        auto extent = Surface::extent(*swapchain.surface_);

        VkSurfaceCapabilitiesKHR surfaceCapabilities;

        if (vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &surfaceCapabilities) != VK_SUCCESS) {
            swapchain.extent_ = {
                std::numeric_limits<std::uint32_t>::max(),
                std::numeric_limits<std::uint32_t>::max(),
            };

            return {};
        }

        if (surfaceCapabilities.currentExtent.width != std::numeric_limits<std::uint32_t>::max()) {
            swapchain.extent_ = surfaceCapabilities.currentExtent;
        }
        else {
            auto& minExtent = surfaceCapabilities.minImageExtent;
            auto& maxExtent = surfaceCapabilities.maxImageExtent;

            swapchain.extent_.width = std::clamp(extent.x, minExtent.width, maxExtent.width);
            swapchain.extent_.height = std::clamp(extent.y, minExtent.height, maxExtent.height);
        }

        return surfaceCapabilities;
    }

    void Swapchain::selectSurfaceFormat(Swapchain& swapchain) {
        auto& physicalDevice = swapchain.instance_->physicalDevice_;
        auto& surface = swapchain.surface_->surface_;

        std::uint32_t formatCount = 0;

        if (vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr) != VK_SUCCESS) {
            throw std::runtime_error("Call failed: renderer::Swapchain::create(): Failed to get surface formats");
        }

        std::vector<VkSurfaceFormatKHR> formats(formatCount);

        if (vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, formats.data()) != VK_SUCCESS) {
            throw std::runtime_error("Call failed: renderer::Swapchain::create(): Failed to get surface formats");
        }

        swapchain.surfaceFormat_ = formats[0];

        if (formats.size() == 1 && formats[0].format == VK_FORMAT_UNDEFINED) {
            swapchain.surfaceFormat_.format = VK_FORMAT_B8G8R8A8_SRGB;
            swapchain.surfaceFormat_.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
        }
        else {
            for (const auto& format : formats) {
                if (format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                    swapchain.surfaceFormat_ = format;
                    break;
                }
            }
        }
    }

    void Swapchain::selectPresentMode(Swapchain& swapchain) {
        auto& physicalDevice = swapchain.instance_->physicalDevice_;
        auto& surface = swapchain.surface_->surface_;

        std::uint32_t presentModeCount = 0;

        if (vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr) != VK_SUCCESS) {
            swapchain.presentMode_ = VK_PRESENT_MODE_MAX_ENUM_KHR;

            return;
        }

        std::vector<VkPresentModeKHR> presentModes(presentModeCount);

        if (vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, presentModes.data()) != VK_SUCCESS) {
            swapchain.presentMode_ = VK_PRESENT_MODE_MAX_ENUM_KHR;

            return;
        }

        swapchain.presentMode_ = VK_PRESENT_MODE_FIFO_KHR;

        std::int32_t chosenPriority = -1;

        for (const auto& mode : presentModes) {
            std::int32_t priority = -1;

            if (mode == VK_PRESENT_MODE_MAILBOX_KHR) {
                priority = 3;
            }
            else if (!swapchain.synchronise_ && mode == VK_PRESENT_MODE_IMMEDIATE_KHR) {
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
                swapchain.presentMode_ = mode;
            }
        }
    }
}