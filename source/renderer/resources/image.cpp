#include <renderer/resources/image.hpp>

#include <renderer/device.hpp>

#include <stdexcept>

namespace renderer {
    Image::Image(const ImageCreateInfo& createInfo)
        : device_(createInfo.device), extent_(createInfo.extent), sampleCount_(createInfo.sampleCount),
          mipLevels_(createInfo.mipLevels), arrayLayers_(createInfo.arrayLayers), type_(mapType(createInfo.type)),
          format_(mapFormat(createInfo.format)), usageFlags_(mapFlags(createInfo.usageFlags)) {
        VkImageCreateInfo imageCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .imageType = type_,
            .format = format_,
            .extent = {createInfo.extent.width, createInfo.extent.height, createInfo.extent.depth},
            .mipLevels = createInfo.mipLevels,
            .arrayLayers = createInfo.arrayLayers,
            .samples = static_cast<VkSampleCountFlagBits>(createInfo.sampleCount),
            .tiling = VK_IMAGE_TILING_OPTIMAL,
            .usage = usageFlags_,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .queueFamilyIndexCount = 0,
            .pQueueFamilyIndices = nullptr,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        };

        if (vkCreateImage(device_->getVkDevice(), &imageCreateInfo, nullptr, &image_) != VK_SUCCESS) {
            throw std::runtime_error("Error constructing renderer::Image: Failed to create image");
        }
    }

    Image::~Image() {
        if (image_ != VK_NULL_HANDLE) {
            vkDestroyImage(device_->getVkDevice(), image_, nullptr);
        }
    }

    ImageFormat Image::getFormat() const {
        return reverseMapFormat(format_);
    }

    ImageType Image::getType() const {
        return reverseMapType(type_);
    }

    data::Extent3D<std::uint32_t> Image::getExtent() const {
        return extent_;
    }

    std::uint32_t Image::getSampleCount() const {
        return sampleCount_;
    }

    std::uint32_t Image::getMipLevels() const {
        return mipLevels_;
    }

    std::uint32_t Image::getArrayLayers() const {
        return arrayLayers_;
    }

    std::uint32_t Image::getUsageFlags() const {
        return reverseMapFlags(usageFlags_);
    }

    VkImage& Image::getVkImage() {
        return image_;
    }

    VkDeviceMemory& Image::getVkDeviceMemory() {
        return deviceMemory_;
    }

    const VkImage& Image::getVkImage() const {
        return image_;
    }

    const VkDeviceMemory& Image::getVkDeviceMemory() const {
        return deviceMemory_;
    }

    VkImageType Image::getVkImageType() const {
        return type_;
    }

    VkFormat Image::getVkFormat() const {
        return format_;
    }

    VkImageUsageFlags Image::getVkImageUsageFlags() const {
        return usageFlags_;
    }

    Image::Image(Device& device)
        : device_(device) {
    }

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

    VkImageUsageFlags Image::mapFlags(std::uint32_t usageFlags) {
        VkImageUsageFlags flags = 0;

        if ((usageFlags & ImageUsageFlags::TRANSFER_SOURCE) == ImageUsageFlags::TRANSFER_SOURCE) {
            flags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        }

        if ((usageFlags & ImageUsageFlags::TRANSFER_DESTINATION) == ImageUsageFlags::TRANSFER_DESTINATION) {
            flags |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        }

        if ((usageFlags & ImageUsageFlags::SAMPLED) == ImageUsageFlags::SAMPLED) {
            flags |= VK_IMAGE_USAGE_SAMPLED_BIT;
        }

        if ((usageFlags & ImageUsageFlags::STORAGE) == ImageUsageFlags::STORAGE) {
            flags |= VK_IMAGE_USAGE_STORAGE_BIT;
        }

        if ((usageFlags & ImageUsageFlags::COLOR_ATTACHMENT) == ImageUsageFlags::COLOR_ATTACHMENT) {
            flags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        }

        if ((usageFlags & ImageUsageFlags::DEPTH_ATTACHMENT) == ImageUsageFlags::DEPTH_ATTACHMENT) {
            flags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        }

        if ((usageFlags & ImageUsageFlags::STENCIL_ATTACHMENT) == ImageUsageFlags::STENCIL_ATTACHMENT) {
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

    ImageType Image::reverseMapType(VkImageType type) {
        switch (type) {
            case VK_IMAGE_TYPE_1D:
                return ImageType::IMAGE_1D;

            case VK_IMAGE_TYPE_2D:
                return ImageType::IMAGE_2D;

            case VK_IMAGE_TYPE_3D:
                return ImageType::IMAGE_3D;

            default:
                throw std::runtime_error("Error calling renderer::ImageView::reverseMapType(): Invalid image view type");
        }
    }

    std::uint32_t Image::reverseMapFlags(VkImageUsageFlags vkFlags) {
        std::uint32_t usageFlags = 0;

        if (vkFlags & VK_IMAGE_USAGE_TRANSFER_SRC_BIT) {
            usageFlags |= ImageUsageFlags::TRANSFER_SOURCE;
        }
        if (vkFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT) {
            usageFlags |= ImageUsageFlags::TRANSFER_DESTINATION;
        }
        if (vkFlags & VK_IMAGE_USAGE_SAMPLED_BIT) {
            usageFlags |= ImageUsageFlags::SAMPLED;
        }
        if (vkFlags & VK_IMAGE_USAGE_STORAGE_BIT) {
            usageFlags |= ImageUsageFlags::STORAGE;
        }
        if (vkFlags & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) {
            usageFlags |= ImageUsageFlags::COLOR_ATTACHMENT;
        }
        if (vkFlags & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) {
            usageFlags |= ImageUsageFlags::DEPTH_ATTACHMENT | ImageUsageFlags::STENCIL_ATTACHMENT;
        }

        return usageFlags;
    }

    ImageView::ImageView(const ImageViewCreateInfo& createInfo)
        : image_(createInfo.image), device_(createInfo.device) {
        VkImageViewCreateInfo viewCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .image = createInfo.image.getVkImage(),
            .viewType = ImageView::mapType(createInfo.type),
            .format = createInfo.image.getVkFormat(),
            .components = {},
            .subresourceRange = {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = createInfo.baseMipLevel,
                .levelCount = createInfo.levelCount,
                .baseArrayLayer = createInfo.baseArrayLayer,
                .layerCount = createInfo.layerCount,
            },
        };

        if (vkCreateImageView(device_->getVkDevice(), &viewCreateInfo, nullptr, &imageView_) != VK_SUCCESS) {
            throw std::runtime_error("Error constructing renderer::ImageView: Failed to create image view");
        }
    }

    ImageView::~ImageView() {
        if (imageView_ != VK_NULL_HANDLE) {
            vkDestroyImageView(device_->getVkDevice(), imageView_, nullptr);
        }
    }

    const Image& ImageView::getImage() const {
        return image_;
    }

    ImageViewType ImageView::getImageViewType() const {
        return reverseMapType(type_);
    }

    VkImageView& ImageView::getVkImageView() {
        return imageView_;
    }

    const VkImageView& ImageView::getVkImageView() const {
        return imageView_;
    }

    VkImageViewType ImageView::getVkImageViewType() const {
        return type_;
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

    ImageViewType ImageView::reverseMapType(VkImageViewType type) {
        switch (type) {
            case VK_IMAGE_VIEW_TYPE_1D:
                return ImageViewType::IMAGE_1D;

            case VK_IMAGE_VIEW_TYPE_2D:
                return ImageViewType::IMAGE_2D;

            case VK_IMAGE_VIEW_TYPE_3D:
                return ImageViewType::IMAGE_3D;

            default:
                throw std::runtime_error("Error calling renderer::ImageView::reverseMapType(): Invalid image view type");
        }
    }
}