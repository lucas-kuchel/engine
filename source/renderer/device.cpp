#include <renderer/device.hpp>
#include <renderer/instance.hpp>
#include <renderer/queue.hpp>
#include <renderer/surface.hpp>

#include <stdexcept>

namespace renderer {
    Device::Device() {
    }

    Device::~Device() {
        if (data_.device != VK_NULL_HANDLE) {
            vkDestroyDevice(data_.device, nullptr);

            data_.device = VK_NULL_HANDLE;
        }
    }

    void Device::create(const DeviceCreateInfo& createInfo) {
        auto& instanceData = createInfo.instance.getData();

        std::vector<std::uint32_t> familyIndexMappings;
        std::vector<std::vector<float>> familyIndexPriorities;

        for (auto& queue : createInfo.queues) {
            auto& queueData = queue.get().getData();

            auto& index = queueData.familyIndex;

            if (familyIndexMappings.size() <= index) {
                familyIndexMappings.resize(index + 1, 0);
                familyIndexPriorities.resize(index + 1);
            }

            if (familyIndexMappings[index] < instanceData.queueFamilyProperties[index].queueCount) {
                familyIndexMappings[index]++;
                familyIndexPriorities[index].push_back(1.0f);
            }
        }

        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

        for (std::uint32_t i = 0; i < familyIndexMappings.size(); i++) {
            if (familyIndexMappings[i] == 0 || familyIndexPriorities[i].size() == 0) {
                continue;
            }

            VkDeviceQueueCreateInfo queueCreateInfo = {
                .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .queueFamilyIndex = i,
                .queueCount = familyIndexMappings[i],
                .pQueuePriorities = familyIndexPriorities[i].data(),
            };

            queueCreateInfos.push_back(queueCreateInfo);
        }

        std::uint32_t extensionCount = 0;

        if (vkEnumerateDeviceExtensionProperties(instanceData.physicalDevice, nullptr, &extensionCount, nullptr) != VK_SUCCESS) {
            throw std::runtime_error("Error calling renderer::Device::create(): Failed to enumerate device extensions");
        }

        std::vector<VkExtensionProperties> extensionProperties(extensionCount);

        if (vkEnumerateDeviceExtensionProperties(instanceData.physicalDevice, nullptr, &extensionCount, extensionProperties.data()) != VK_SUCCESS) {
            throw std::runtime_error("Error calling renderer::Device::create(): Failed to enumerate device extensions");
        }

        std::vector<const char*> selectedExtensions;

        bool foundSwapchain = false;

        for (auto& extensionInfo : extensionProperties) {
            bool match = false;

            match |= std::string_view(extensionInfo.extensionName) == "VK_KHR_swapchain";

            if (match) {
                foundSwapchain = true;

                selectedExtensions.push_back(extensionInfo.extensionName);
            }
        }

        if (!foundSwapchain) {
            throw std::runtime_error("Error calling renderer::Device::create(): Swapchain is unsupported on this system");
        }

        std::uint32_t extensionInfoCount = selectedExtensions.size();
        std::uint32_t queueCreateInfoCount = queueCreateInfos.size();

        VkDeviceCreateInfo deviceCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .queueCreateInfoCount = queueCreateInfoCount,
            .pQueueCreateInfos = queueCreateInfos.data(),
            .enabledLayerCount = 0,
            .ppEnabledLayerNames = nullptr,
            .enabledExtensionCount = extensionInfoCount,
            .ppEnabledExtensionNames = selectedExtensions.data(),
            .pEnabledFeatures = nullptr,
        };

        if (vkCreateDevice(instanceData.physicalDevice, &deviceCreateInfo, nullptr, &data_.device) != VK_SUCCESS) {
            throw std::runtime_error("Error calling renderer::Device::create(): Failed to create device");
        }

        for (auto& queue : createInfo.queues) {
            auto& queueData = queue.get().getData();

            vkGetDeviceQueue(data_.device, queueData.familyIndex, queueData.queueIndex, &queueData.queue);
        }
    }

    ImageHandle Device::createImage(const ImageCreateInfo& createInfo) {
        Image image;

        VkImageCreateInfo imageCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .imageType = Image::mapType(createInfo.type),
            .format = Image::mapFormat(createInfo.format),
            .extent = {createInfo.extent.width, createInfo.extent.height, createInfo.extent.depth},
            .mipLevels = createInfo.mipLevels,
            .arrayLayers = createInfo.arrayLayers,
            .samples = static_cast<VkSampleCountFlagBits>(createInfo.sampleCount),
            .tiling = Image::mapTiling(createInfo.tiling),
            .usage = Image::mapFlags(createInfo.usage),
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .queueFamilyIndexCount = 0,
            .pQueueFamilyIndices = nullptr,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        };

        if (vkCreateImage(data_.device, &imageCreateInfo, nullptr, &image.image) != VK_SUCCESS) {
            throw std::runtime_error("Error calling renderer::Device::createImage(): Failed to create image");
        }

        return data_.images_.insert(image);
    }

    ImageViewHandle Device::createImageView(const ImageViewCreateInfo& createInfo) {
        ImageView imageView;

        const auto& image = data_.images_.get(createInfo.image);

        VkImageViewCreateInfo viewCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .image = image.image,
            .viewType = ImageView::mapType(createInfo.viewType),
            .format = Image::mapFormat(createInfo.format),
            .components = {},
            .subresourceRange = {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = createInfo.baseMipLevel,
                .levelCount = createInfo.levelCount,
                .baseArrayLayer = createInfo.baseArrayLayer,
                .layerCount = createInfo.layerCount,
            },
        };

        if (vkCreateImageView(data_.device, &viewCreateInfo, nullptr, &imageView.imageView) != VK_SUCCESS) {
            throw std::runtime_error("Error calling renderer::Device::createImageView(): Failed to create image view");
        }

        return data_.imageViews_.insert(imageView);
    }

    void Device::destroyImage(ImageHandle handle) {
        auto& image = data_.images_.get(handle);

        if (image.image != VK_NULL_HANDLE) {
            vkDestroyImage(data_.device, image.image, nullptr);
        }

        data_.images_.remove(handle);
    }

    void Device::destroyImageView(ImageViewHandle handle) {
        auto& view = data_.imageViews_.get(handle);

        if (view.imageView != VK_NULL_HANDLE) {
            vkDestroyImageView(data_.device, view.imageView, nullptr);
        }

        data_.imageViews_.remove(handle);
    }

    DeviceData& Device::getData() {
        return data_;
    }
}