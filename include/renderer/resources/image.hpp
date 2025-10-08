#pragma once

#include <renderer/resources/config.hpp>

#include <data/extent.hpp>
#include <data/references.hpp>

#include <cstdint>
#include <span>

#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

namespace renderer {
    class Queue;
    class Device;
    class Instance;

    // @brief Creation information for the image
    struct ImageCreateInfo {
        Device& device;
        ImageType type;
        ImageFormat format;
        MemoryType memoryType;
        Flags usageFlags;

        data::Extent3D<std::uint32_t> extent;

        std::uint32_t sampleCount;
        std::uint32_t mipLevels;
        std::uint32_t arrayLayers;
    };

    // @brief Represents an image
    // @note Not safe to copy
    class Image {
    public:
        Image(const ImageCreateInfo& createInfo);
        ~Image();

        Image(const Image&) = delete;
        Image(Image&&) noexcept = default;

        Image& operator=(const Image&) = delete;
        Image& operator=(Image&&) noexcept = default;

        std::span<std::uint8_t> map(std::uint64_t sizeBytes, std::uint64_t offsetBytes);
        void unmap();

        [[nodiscard]] bool mapped() const;
        [[nodiscard]] bool mappable() const;

        [[nodiscard]] ImageFormat format() const;
        [[nodiscard]] ImageType type() const;
        [[nodiscard]] std::uint64_t size() const;

        [[nodiscard]] data::Extent3D<std::uint32_t> extent() const;

        [[nodiscard]] std::uint32_t sampleCount() const;
        [[nodiscard]] std::uint32_t mipLevels() const;
        [[nodiscard]] std::uint32_t arrayLayers() const;
        [[nodiscard]] std::uint32_t usageFlags() const;

        [[nodiscard]] Device& device();
        [[nodiscard]] const Device& device() const;

        [[nodiscard]] VkImage& getVkImage();
        [[nodiscard]] const VkImage& getVkImage() const;

        [[nodiscard]] VkImageType getVkImageType() const;
        [[nodiscard]] VkFormat getVkFormat() const;

    private:
        Image(Device& device);

        data::Ref<Device> device_;
        data::Extent3D<std::uint32_t> extent_;

        std::uint32_t sampleCount_;
        std::uint32_t mipLevels_;
        std::uint32_t arrayLayers_;

        VkImage image_ = VK_NULL_HANDLE;
        VmaAllocation memory_ = VK_NULL_HANDLE;
        VkImageType type_ = VK_IMAGE_TYPE_MAX_ENUM;
        VkFormat format_ = VK_FORMAT_MAX_ENUM;

        bool hostVisible_ = false;
        bool hostCoherent_ = false;
        bool mapped_ = false;

        std::uint64_t mapSize_ = 0;
        std::uint64_t mapOffset_ = 0;
        std::uint64_t size_ = 0;

        static VkFormat mapFormat(ImageFormat format, renderer::Instance& instance);
        static ImageFormat reverseMapFormat(VkFormat format);

        static VkImageType mapType(ImageType type);
        static ImageType reverseMapType(VkImageType type);

        friend class Swapchain;
        friend class RenderPass;
    };

    struct BufferImageCopyRegion {
        std::uint64_t bufferOffset;
        std::uint32_t bufferRowLength;
        std::uint32_t bufferImageHeight;
        std::uint32_t mipLevel;
        std::uint32_t baseArrayLayer;
        std::uint32_t arrayLayerCount;

        Flags imageAspectMask;

        data::Extent3D<std::int32_t> imageOffset;
        data::Extent3D<std::uint32_t> imageExtent;
    };

    struct ImageMemoryBarrier {
        Image& image;
        data::NullableRef<Queue> sourceQueue;
        data::NullableRef<Queue> destinationQueue;

        ImageLayout oldLayout;
        ImageLayout newLayout;

        std::uint32_t baseMipLevel;
        std::uint32_t mipLevelCount;
        std::uint32_t baseArrayLayer;
        std::uint32_t arrayLayerCount;

        Flags aspectMask;
        Flags sourceAccessFlags;
        Flags destinationAccessFlags;
    };

    // @brief Creation information for an image view
    struct ImageViewCreateInfo {
        Image& image;
        ImageViewType type;
        Flags aspectFlags;

        std::uint32_t baseMipLevel;
        std::uint32_t levelCount;
        std::uint32_t baseArrayLayer;
        std::uint32_t layerCount;
    };

    // @brief Represents a usable view of an image
    // @note Not safe to copy
    class ImageView {
    public:
        ImageView(const ImageViewCreateInfo& createInfo);
        ~ImageView();

        ImageView(const ImageView&) = delete;
        ImageView(ImageView&&) noexcept = default;

        ImageView& operator=(const ImageView&) = delete;
        ImageView& operator=(ImageView&&) noexcept = default;

        [[nodiscard]] const Image& image() const;

        [[nodiscard]] ImageViewType type() const;

        [[nodiscard]] std::uint32_t baseMipLevel() const;
        [[nodiscard]] std::uint32_t levelCount() const;
        [[nodiscard]] std::uint32_t baseArrayLayer() const;
        [[nodiscard]] std::uint32_t layerCount() const;

        [[nodiscard]] VkImageView& getVkImageView();
        [[nodiscard]] const VkImageView& getVkImageView() const;

        [[nodiscard]] VkImageViewType getVkImageViewType() const;

    private:
        data::Ref<Image> image_;

        VkImageView imageView_ = VK_NULL_HANDLE;
        VkImageViewType type_ = VK_IMAGE_VIEW_TYPE_MAX_ENUM;

        std::uint32_t baseMipLevel_ = 0;
        std::uint32_t levelCount_ = 0;
        std::uint32_t baseArrayLayer_ = 0;
        std::uint32_t layerCount_ = 0;

        static VkImageViewType mapType(ImageViewType type);
        static ImageViewType reverseMapType(VkImageViewType type);
    };
}