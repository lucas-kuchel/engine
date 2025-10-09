#include <renderer/resources/image.hpp>

#include <renderer/device.hpp>
#include <renderer/instance.hpp>

#include <stdexcept>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace renderer {
    Image::Image(const ImageCreateInfo& createInfo)
        : device_(createInfo.device), extent_(createInfo.extent), sampleCount_(createInfo.sampleCount),
          mipLevels_(createInfo.mipLevels), arrayLayers_(createInfo.arrayLayers),
          type_(mapType(createInfo.type)), format_(mapFormat(createInfo.format, createInfo.device.getInstance())) {
        VmaMemoryUsage memoryUsage;
        VkMemoryPropertyFlags memoryProperties;

        switch (createInfo.memoryType) {
            case MemoryType::DEVICE_LOCAL:
                memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY;
                memoryProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
                break;

            case MemoryType::HOST_VISIBLE:
                memoryUsage = VMA_MEMORY_USAGE_CPU_TO_GPU;
                memoryProperties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
                break;
        }

        VmaAllocationCreateInfo allocationCreateInfo = {
            .flags = 0,
            .usage = memoryUsage,
            .requiredFlags = 0,
            .preferredFlags = memoryProperties,
            .memoryTypeBits = 0,
            .pool = nullptr,
            .pUserData = nullptr,
            .priority = 0.0,
        };

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
            .usage = ImageUsageFlags::mapFrom(createInfo.usageFlags),
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .queueFamilyIndexCount = 0,
            .pQueueFamilyIndices = nullptr,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        };

        VmaAllocationInfo allocationInfo = {};

        if (vmaCreateImage(device_->getVmaAllocator(), &imageCreateInfo, &allocationCreateInfo, &image_, &memory_, &allocationInfo) != VK_SUCCESS) {
            throw std::runtime_error("Construction failed: renderer::Image: Failed to create image");
        }

        auto& deviceMemoryProperties = device_->getInstance().getVkPhysicalDeviceMemoryProperties();

        VkMemoryPropertyFlags properties = deviceMemoryProperties.memoryTypes[allocationInfo.memoryType].propertyFlags;

        hostVisible_ = (properties & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) != 0;
        hostCoherent_ = (properties & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) != 0;

        size_ = allocationInfo.size;
    }

    Image::~Image() {
        auto& device = device_.get();

        if (mapped_) {

            if (hostCoherent_) {
                vmaFlushAllocation(device.getVmaAllocator(), memory_, mapOffset_, mapSize_);
            }

            vmaUnmapMemory(device.getVmaAllocator(), memory_);

            mapped_ = false;
        }

        if (image_ != VK_NULL_HANDLE) {
            vmaDestroyImage(device.getVmaAllocator(), image_, memory_);
        }
    }

    std::span<std::uint8_t> Image::map(std::uint64_t sizeBytes, std::uint64_t offsetBytes) {
        if (!hostVisible_) {
            throw std::runtime_error("Call failed: renderer::Image::map(): Cannot map memory that is not host visible");
        }

        if (mapped_) {
            throw std::runtime_error("Call failed: renderer::Image::map(): Memory is already mapped");
        }

        mapped_ = true;

        auto& properties = device_->getInstance().getVkPhysicalDeviceProperties();

        VkDeviceSize atomSize = properties.limits.nonCoherentAtomSize;

        mapOffset_ = offsetBytes & ~(atomSize - 1);
        mapSize_ = ((mapOffset_ + sizeBytes + atomSize - 1) & ~(atomSize - 1)) - mapOffset_;

        if (!hostCoherent_) {
            vmaInvalidateAllocation(device_->getVmaAllocator(), memory_, mapOffset_, mapSize_);
        }

        void* data = nullptr;

        vmaMapMemory(device_->getVmaAllocator(), memory_, &data);

        return {reinterpret_cast<std::uint8_t*>(data) + mapOffset_, mapSize_};
    }

    void Image::unmap() {
        if (!hostCoherent_) {
            vmaFlushAllocation(device_->getVmaAllocator(), memory_, mapOffset_, mapSize_);
        }

        if (mapped_) {
            vmaUnmapMemory(device_->getVmaAllocator(), memory_);

            mapped_ = false;
        }
    }

    bool Image::mapped() const {
        return mapped_;
    }

    bool Image::mappable() const {
        return hostVisible_ || hostCoherent_;
    }

    ImageFormat Image::format() const {
        return reverseMapFormat(format_);
    }

    ImageType Image::type() const {
        return reverseMapType(type_);
    }

    data::Extent3D<std::uint32_t> Image::extent() const {
        return extent_;
    }

    std::uint64_t Image::size() const {
        return size_;
    }

    Device& Image::device() {
        return device_.get();
    }

    const Device& Image::device() const {
        return device_.get();
    }

    std::uint32_t Image::sampleCount() const {
        return sampleCount_;
    }

    std::uint32_t Image::mipLevels() const {
        return mipLevels_;
    }

    std::uint32_t Image::arrayLayers() const {
        return arrayLayers_;
    }

    VkImage& Image::getVkImage() {
        return image_;
    }

    const VkImage& Image::getVkImage() const {
        return image_;
    }

    VkImageType Image::getVkImageType() const {
        return type_;
    }

    VkFormat Image::getVkFormat() const {
        return format_;
    }

    Image::Image(Device& device)
        : device_(device) {
    }

    VkFormat Image::mapFormat(ImageFormat format, renderer::Instance& instance) {
        auto findSupportedFormat = [&](const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) -> VkFormat {
            for (VkFormat vkFormat : candidates) {
                VkFormatProperties properties;

                vkGetPhysicalDeviceFormatProperties(instance.getVkPhysicalDevice(), vkFormat, &properties);

                if (tiling == VK_IMAGE_TILING_LINEAR && (properties.linearTilingFeatures & features) == features) {
                    return vkFormat;
                }
                if (tiling == VK_IMAGE_TILING_OPTIMAL && (properties.optimalTilingFeatures & features) == features) {
                    return vkFormat;
                }
            }

            throw std::runtime_error("Call failed: renderer::Image::mapFormat(): Failed to find supported non-colour image format");
        };

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

            case ImageFormat::DEPTH_ONLY: {
                std::vector<VkFormat> candidates = {
                    VK_FORMAT_D32_SFLOAT,
                    VK_FORMAT_D32_SFLOAT_S8_UINT,
                    VK_FORMAT_D24_UNORM_S8_UINT,
                    VK_FORMAT_D16_UNORM,
                    VK_FORMAT_D16_UNORM_S8_UINT,
                };

                return findSupportedFormat(candidates, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
            }

            case ImageFormat::STENCIL_ONLY: {
                std::vector<VkFormat> candidates = {
                    VK_FORMAT_S8_UINT,
                };

                return findSupportedFormat(candidates, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
            }

            case ImageFormat::DEPTH_STENCIL: {
                std::vector<VkFormat> candidates = {
                    VK_FORMAT_D32_SFLOAT_S8_UINT,
                    VK_FORMAT_D24_UNORM_S8_UINT,
                    VK_FORMAT_D16_UNORM_S8_UINT,
                };

                return findSupportedFormat(candidates, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
            }

            default:
                throw std::runtime_error("Call failed: renderer::Image::mapFormat(): Invalid image format");
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
                throw std::runtime_error("Call failed: renderer::Image::mapType(): Invalid image type");
        }
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

            case VK_FORMAT_D32_SFLOAT:
            case VK_FORMAT_D16_UNORM:
                return ImageFormat::DEPTH_ONLY;

            case VK_FORMAT_S8_UINT:
                return ImageFormat::STENCIL_ONLY;

            case VK_FORMAT_D16_UNORM_S8_UINT:
            case VK_FORMAT_D24_UNORM_S8_UINT:
            case VK_FORMAT_D32_SFLOAT_S8_UINT:
                return ImageFormat::DEPTH_STENCIL;

            default:
                throw std::runtime_error("Call failed: renderer::Image::reverseMapFormat(): Invalid image format");
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
                throw std::runtime_error("Call failed: renderer::ImageView::reverseMapType(): Invalid image view type");
        }
    }

    ImageView::ImageView(const ImageViewCreateInfo& createInfo)
        : image_(createInfo.image) {
        VkImageViewCreateInfo viewCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .image = createInfo.image.getVkImage(),
            .viewType = ImageView::mapType(createInfo.type),
            .format = createInfo.image.getVkFormat(),
            .components = {},
            .subresourceRange = {
                .aspectMask = ImageAspectFlags::mapFrom(createInfo.aspectFlags),
                .baseMipLevel = createInfo.baseMipLevel,
                .levelCount = createInfo.levelCount,
                .baseArrayLayer = createInfo.baseArrayLayer,
                .layerCount = createInfo.layerCount,
            },
        };

        if (vkCreateImageView(image_->device().getVkDevice(), &viewCreateInfo, nullptr, &imageView_) != VK_SUCCESS) {
            throw std::runtime_error("Construction failed: renderer::ImageView: Failed to create image view");
        }
    }

    ImageView::~ImageView() {
        if (imageView_ != VK_NULL_HANDLE) {
            vkDestroyImageView(image_->device().getVkDevice(), imageView_, nullptr);
        }
    }

    const Image& ImageView::image() const {
        return image_.get();
    }

    ImageViewType ImageView::type() const {
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
                throw std::runtime_error("Call failed: renderer::ImageView::mapType(): Invalid image view type");
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
                throw std::runtime_error("Call failed: renderer::ImageView::reverseMapType(): Invalid image view type");
        }
    }
}