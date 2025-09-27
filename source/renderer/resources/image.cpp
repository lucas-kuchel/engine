#include <renderer/resources/image.hpp>

#include <stdexcept>

namespace renderer {
    VkFormat Image::mapFormat(ImageFormat format) {
        switch (format) {
            case ImageFormat::R8_UNORM:
                return VK_FORMAT_R8_UNORM;

            case ImageFormat::R8G8_UNORM:
                return VK_FORMAT_R8G8_UNORM;

            case ImageFormat::R8G8B8_UNORM:
                return VK_FORMAT_R8G8B8_UNORM;

            case ImageFormat::R8G8B8A8_UNORM:
                return VK_FORMAT_R8G8B8A8_UNORM;

            case ImageFormat::B8G8R8A8_UNORM:
                return VK_FORMAT_B8G8R8A8_UNORM;

            case ImageFormat::R16G16B16A16_SFLOAT:
                return VK_FORMAT_R16G16B16A16_SFLOAT;

            case ImageFormat::R32G32B32A32_SFLOAT:
                return VK_FORMAT_R32G32B32A32_SFLOAT;

            case ImageFormat::B8G8R8A8_SRGB:
                return VK_FORMAT_B8G8R8A8_SRGB;

            case ImageFormat::D16_UNORM:
                return VK_FORMAT_D16_UNORM;

            case ImageFormat::D24_UNORM_S8_UINT:
                return VK_FORMAT_D24_UNORM_S8_UINT;

            case ImageFormat::D32_SFLOAT:
                return VK_FORMAT_D32_SFLOAT;

            case ImageFormat::D32_SFLOAT_S8_UINT:
                return VK_FORMAT_D32_SFLOAT_S8_UINT;

            default:
                throw std::runtime_error("Error calling renderer::Image::mapFormat(): Invalid image format");
        }
    }

    VkImageType Image::mapType(ImageType type) {
        switch (type) {
            case ImageType::IMAGE_1D:
                return VK_IMAGE_TYPE_1D;

            case ImageType::IMAGE_2D:
                return VK_IMAGE_TYPE_2D;

            case ImageType::IMAGE_3D:
                return VK_IMAGE_TYPE_3D;

            default:
                throw std::runtime_error("Error calling renderer::Image::mapType(): Invalid image type");
        }
    }

    VkImageTiling Image::mapTiling(ImageTiling tiling) {
        switch (tiling) {
            case ImageTiling::OPTIMAL:
                return VK_IMAGE_TILING_OPTIMAL;

            case ImageTiling::LINEAR:
                return VK_IMAGE_TILING_LINEAR;

            default:
                throw std::runtime_error("Error calling renderer::Image::mapTiling(): Invalid image tiling");
        }
    }

    VkImageUsageFlags Image::mapFlags(ImageUsage usage) {
        VkImageUsageFlags flags = 0;

        if ((usage & ImageUsage::TRANSFER_SOURCE) == ImageUsage::TRANSFER_SOURCE) {
            flags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        }

        if ((usage & ImageUsage::TRANSFER_DESTINATION) == ImageUsage::TRANSFER_DESTINATION) {
            flags |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        }

        if ((usage & ImageUsage::SAMPLED) == ImageUsage::SAMPLED) {
            flags |= VK_IMAGE_USAGE_SAMPLED_BIT;
        }

        if ((usage & ImageUsage::STORAGE) == ImageUsage::STORAGE) {
            flags |= VK_IMAGE_USAGE_STORAGE_BIT;
        }

        if ((usage & ImageUsage::COLOR_ATTACHMENT) == ImageUsage::COLOR_ATTACHMENT) {
            flags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        }

        if ((usage & ImageUsage::DEPTH_ATTACHMENT) == ImageUsage::DEPTH_ATTACHMENT) {
            flags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        }

        if ((usage & ImageUsage::STENCIL_ATTACHMENT) == ImageUsage::STENCIL_ATTACHMENT) {
            flags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        }

        return flags;
    }

    ImageFormat Image::reverseMapFormat(VkFormat format) {
        switch (format) {
            case VK_FORMAT_R8_UNORM:
                return ImageFormat::R8_UNORM;

            case VK_FORMAT_R8G8_UNORM:
                return ImageFormat::R8G8_UNORM;

            case VK_FORMAT_R8G8B8_UNORM:
                return ImageFormat::R8G8B8_UNORM;

            case VK_FORMAT_R8G8B8A8_UNORM:
                return ImageFormat::R8G8B8A8_UNORM;

            case VK_FORMAT_B8G8R8A8_UNORM:
                return ImageFormat::B8G8R8A8_UNORM;

            case VK_FORMAT_R16G16B16A16_SFLOAT:
                return ImageFormat::R16G16B16A16_SFLOAT;

            case VK_FORMAT_R32G32B32A32_SFLOAT:
                return ImageFormat::R32G32B32A32_SFLOAT;

            case VK_FORMAT_B8G8R8A8_SRGB:
                return ImageFormat::B8G8R8A8_SRGB;

            case VK_FORMAT_D16_UNORM:
                return ImageFormat::D16_UNORM;

            case VK_FORMAT_D24_UNORM_S8_UINT:
                return ImageFormat::D24_UNORM_S8_UINT;

            case VK_FORMAT_D32_SFLOAT:
                return ImageFormat::D32_SFLOAT;

            case VK_FORMAT_D32_SFLOAT_S8_UINT:
                return ImageFormat::D32_SFLOAT_S8_UINT;

            default:
                throw std::runtime_error("Error calling renderer::Image::reverseMapFormat(): Invalid image format");
        }
    }

    VkImageViewType ImageView::mapType(ImageViewType type) {
        switch (type) {
            case ImageViewType::IMAGE_1D:
                return VK_IMAGE_VIEW_TYPE_1D;

            case ImageViewType::IMAGE_2D:
                return VK_IMAGE_VIEW_TYPE_2D;

            case ImageViewType::IMAGE_3D:
                return VK_IMAGE_VIEW_TYPE_3D;

            default:
                throw std::runtime_error("Error calling renderer::ImageView::mapType(): Invalid image view type");
        }
    }
}