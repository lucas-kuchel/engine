#pragma once

#include <data/extent.hpp>
#include <data/registry.hpp>

#include <vulkan/vulkan.h>

namespace renderer {
    enum class ImageType {
        IMAGE_1D,
        IMAGE_2D,
        IMAGE_3D,
    };

    enum class ImageFormat {
        R8_UNORM,
        R8G8_UNORM,
        R8G8B8_UNORM,
        R8G8B8A8_UNORM,
        B8G8R8A8_UNORM,
        B8G8R8A8_SRGB,
        R16G16B16A16_SFLOAT,
        R32G32B32A32_SFLOAT,

        D16_UNORM,
        D24_UNORM_S8_UINT,
        D32_SFLOAT,
        D32_SFLOAT_S8_UINT,
    };

    enum class ImageTiling {
        OPTIMAL,
        LINEAR,
    };

    enum class ImageUsage : uint32_t {
        NONE = 0,
        TRANSFER_SOURCE = 1 << 0,
        TRANSFER_DESTINATION = 1 << 1,
        SAMPLED = 1 << 2,
        STORAGE = 1 << 3,
        COLOR_ATTACHMENT = 1 << 4,
        DEPTH_ATTACHMENT = 1 << 5,
        STENCIL_ATTACHMENT = 1 << 6,
    };

    inline ImageUsage operator|(ImageUsage a, ImageUsage b) {
        return static_cast<ImageUsage>(static_cast<std::uint32_t>(a) | static_cast<std::uint32_t>(b));
    }

    inline ImageUsage& operator|=(ImageUsage& a, ImageUsage b) {
        a = a | b;
        return a;
    }

    inline ImageUsage operator&(ImageUsage a, ImageUsage b) {
        return static_cast<ImageUsage>(static_cast<std::uint32_t>(a) & static_cast<std::uint32_t>(b));
    }

    inline ImageUsage& operator&=(ImageUsage& a, ImageUsage b) {
        a = a & b;
        return a;
    }

    struct ImageCreateInfo {
        ImageType type;
        ImageFormat format;
        ImageTiling tiling;
        ImageUsage usage;

        data::Extent3D extent;

        std::uint32_t sampleCount = 1;
        std::uint32_t mipLevels = 1;
        std::uint32_t arrayLayers = 1;
    };

    struct Image {
        VkImage image = VK_NULL_HANDLE;

        static VkFormat mapFormat(ImageFormat format);
        static VkImageType mapType(ImageType type);
        static VkImageTiling mapTiling(ImageTiling tiling);
        static VkImageUsageFlags mapFlags(ImageUsage usage);

        static ImageFormat reverseMapFormat(VkFormat format);
    };

    using ImageHandle = data::Handle<Image>;

    enum class ImageViewType {
        IMAGE_1D,
        IMAGE_2D,
        IMAGE_3D,
    };

    struct ImageViewCreateInfo {
        ImageHandle image;
        ImageViewType viewType;
        ImageFormat format;

        std::uint32_t baseMipLevel = 0;
        std::uint32_t levelCount = 1;
        std::uint32_t baseArrayLayer = 0;
        std::uint32_t layerCount = 1;
    };

    struct ImageView {
        VkImageView imageView = VK_NULL_HANDLE;

        static VkImageViewType mapType(ImageViewType usage);
    };

    using ImageViewHandle = data::Handle<ImageView>;
}