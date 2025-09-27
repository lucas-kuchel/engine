#pragma once

#include <data/references.hpp>
#include <data/registry.hpp>

#include <renderer/resources/image.hpp>

#include <vulkan/vulkan.h>

namespace renderer {
    class Instance;
    class Surface;
    class Queue;

    struct DeviceData {
        VkDevice device = VK_NULL_HANDLE;

        data::UniqueRegistry<Image> images_;
        data::UniqueRegistry<ImageView> imageViews_;
    };

    struct DeviceCreateInfo {
        Instance& instance;

        data::ReferenceList<Queue> queues;
    };

    class Device {
    public:
        Device();
        ~Device();

        Device(const Device&) = delete;
        Device(Device&&) noexcept = default;

        Device& operator=(const Device&) = delete;
        Device& operator=(Device&&) noexcept = default;

        void create(const DeviceCreateInfo& createInfo);

        ImageHandle createImage(const ImageCreateInfo& createInfo);
        ImageViewHandle createImageView(const ImageViewCreateInfo& createInfo);

        void destroyImage(ImageHandle image);
        void destroyImageView(ImageViewHandle imageView);

        DeviceData& getData();

    private:
        DeviceData data_;
    };
}