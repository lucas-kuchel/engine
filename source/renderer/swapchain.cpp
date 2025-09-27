#include <renderer/device.hpp>
#include <renderer/instance.hpp>
#include <renderer/surface.hpp>
#include <renderer/swapchain.hpp>

#include <app/window.hpp>

#include <algorithm>
#include <limits>
#include <stdexcept>

namespace renderer {
    Swapchain::Swapchain() {
    }

    Swapchain::~Swapchain() {
        if (data_.swapchain != VK_NULL_HANDLE) {
            vkDestroySwapchainKHR(data_.device->getData().device, data_.swapchain, nullptr);

            data_.swapchain = VK_NULL_HANDLE;
        }

        for (auto& imageView : data_.imageViews) {
            data_.device->destroyImageView(imageView);
        }

        data_.device = nullptr;
    }

    void Swapchain::create(const SwapchainCreateInfo& createInfo) {
        auto& instanceData = createInfo.instance.getData();
        auto& surfaceData = createInfo.surface.getData();
        auto& deviceData = createInfo.device.getData();

        data_.device = &createInfo.device;
        data_.imageCount = createInfo.imageCount;
        data_.synchronise = createInfo.synchronise;

        VkSurfaceCapabilitiesKHR surfaceCapabilities;

        if (vkGetPhysicalDeviceSurfaceCapabilitiesKHR(instanceData.physicalDevice, surfaceData.surface, &surfaceCapabilities) != VK_SUCCESS) {
            throw std::runtime_error("Error calling renderer::Swapchain::create(): Failed to enumerate surface capabilities");
        }

        VkExtent2D extent;

        if (surfaceCapabilities.currentExtent.width != std::numeric_limits<std::uint32_t>::max()) {
            extent = surfaceCapabilities.currentExtent;
        }
        else {
            extent.width = std::clamp(surfaceData.window->getExtent().width, surfaceCapabilities.minImageExtent.width, surfaceCapabilities.maxImageExtent.width);
            extent.height = std::clamp(surfaceData.window->getExtent().height, surfaceCapabilities.minImageExtent.height, surfaceCapabilities.maxImageExtent.height);
        }

        std::uint32_t formatCount = 0;

        if (vkGetPhysicalDeviceSurfaceFormatsKHR(instanceData.physicalDevice, surfaceData.surface, &formatCount, nullptr) != VK_SUCCESS) {
            throw std::runtime_error("Error calling renderer::Swapchain::create(): Failed to get surface formats");
        }

        std::vector<VkSurfaceFormatKHR> formats(formatCount);

        if (vkGetPhysicalDeviceSurfaceFormatsKHR(instanceData.physicalDevice, surfaceData.surface, &formatCount, formats.data()) != VK_SUCCESS) {
            throw std::runtime_error("Error calling renderer::Swapchain::create(): Failed to get surface formats");
        }

        VkSurfaceFormatKHR chosenFormat = formats[0];

        if (formats.size() == 1 && formats[0].format == VK_FORMAT_UNDEFINED) {
            chosenFormat.format = VK_FORMAT_B8G8R8A8_SRGB;
            chosenFormat.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
        }
        else {
            for (const auto& format : formats) {
                if (format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                    chosenFormat = format;
                    break;
                }
            }
        }

        std::uint32_t presentModeCount = 0;

        if (vkGetPhysicalDeviceSurfacePresentModesKHR(instanceData.physicalDevice, surfaceData.surface, &presentModeCount, nullptr) != VK_SUCCESS) {
            throw std::runtime_error("Error calling renderer::Swapchain::create(): Failed to get surface present modes");
        }

        std::vector<VkPresentModeKHR> presentModes(presentModeCount);

        if (vkGetPhysicalDeviceSurfacePresentModesKHR(instanceData.physicalDevice, surfaceData.surface, &presentModeCount, presentModes.data()) != VK_SUCCESS) {
            throw std::runtime_error("Error calling renderer::Swapchain::create(): Failed to get surface present modes");
        }

        VkPresentModeKHR chosenPresentMode = VK_PRESENT_MODE_FIFO_KHR;

        std::int32_t chosenPriority = -1;

        for (const auto& mode : presentModes) {
            std::int32_t priority = -1;

            if (mode == VK_PRESENT_MODE_MAILBOX_KHR) {
                priority = 3;
            }
            else if (!createInfo.synchronise && mode == VK_PRESENT_MODE_IMMEDIATE_KHR) {
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
                chosenPresentMode = mode;
            }
        }

        VkSwapchainCreateInfoKHR swapchainCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
            .pNext = nullptr,
            .flags = 0,
            .surface = surfaceData.surface,
            .minImageCount = std::max(std::min(static_cast<std::uint32_t>(createInfo.imageCount), surfaceCapabilities.maxImageCount), surfaceCapabilities.minImageCount),
            .imageFormat = chosenFormat.format,
            .imageColorSpace = chosenFormat.colorSpace,
            .imageExtent = extent,
            .imageArrayLayers = 1,
            .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
            .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .queueFamilyIndexCount = 0,
            .pQueueFamilyIndices = nullptr,
            .preTransform = surfaceCapabilities.currentTransform,
            .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
            .presentMode = chosenPresentMode,
            .clipped = VK_TRUE,
            .oldSwapchain = VK_NULL_HANDLE,
        };

        if (vkCreateSwapchainKHR(deviceData.device, &swapchainCreateInfo, nullptr, &data_.swapchain) != VK_SUCCESS) {
            throw std::runtime_error("Error calling renderer::Swapchain::create(): Failed to create swapchain");
        }

        std::uint32_t actualImageCount = 0;

        if (vkGetSwapchainImagesKHR(data_.device->getData().device, data_.swapchain, &actualImageCount, nullptr) != VK_SUCCESS) {
            throw std::runtime_error("Error calling renderer::Swapchain::create(): Failed to query swapchain images");
        }

        std::vector<VkImage> queriedImages(actualImageCount);

        if (vkGetSwapchainImagesKHR(data_.device->getData().device, data_.swapchain, &actualImageCount, queriedImages.data()) != VK_SUCCESS) {
            throw std::runtime_error("Error calling renderer::Swapchain::create(): Failed to query swapchain images");
        }

        auto& imagesRegistry = deviceData.images_;

        data_.images.reserve(actualImageCount);
        data_.imageViews.reserve(data_.images.size());

        ImageFormat swapchainFormat = Image::reverseMapFormat(chosenFormat.format);

        for (auto image : queriedImages) {
            Image newImage;
            newImage.image = image;

            auto handle = imagesRegistry.insert(newImage);
            data_.images.push_back(handle);

            ImageViewCreateInfo viewInfo = {
                .image = handle,
                .viewType = ImageViewType::IMAGE_2D,
                .format = swapchainFormat,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1,
            };

            auto viewHandle = createInfo.device.createImageView(viewInfo);
            data_.imageViews.push_back(viewHandle);
        }
    }

    SwapchainData& Swapchain::getData() {
        return data_;
    }
}