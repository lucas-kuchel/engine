#pragma once

#include <renderer/commands/buffer.hpp>

#include <data/extent.hpp>

#include <cstdint>

#include <vulkan/vulkan.h>

namespace renderer {
    // @brief Creation information for the image
    struct ImageCreateInfo {
        Device& device;
        ImageType type;
        ImageFormat format;
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

        [[nodiscard]] ImageFormat format() const;
        [[nodiscard]] ImageType type() const;

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

        data::Reference<Device> device_;
        data::Extent3D<std::uint32_t> extent_;

        std::uint32_t sampleCount_;
        std::uint32_t mipLevels_;
        std::uint32_t arrayLayers_;

        VkImage image_ = VK_NULL_HANDLE;
        VkImageType type_ = VK_IMAGE_TYPE_MAX_ENUM;
        VkFormat format_ = VK_FORMAT_MAX_ENUM;

        static VkFormat mapFormat(ImageFormat format);
        static ImageFormat reverseMapFormat(VkFormat format);

        static VkImageType mapType(ImageType type);
        static ImageType reverseMapType(VkImageType type);

        static VkImageUsageFlags mapFlags(std::uint32_t usageFlags);
        static std::uint32_t reverseMapFlags(VkImageUsageFlags usageFlags);

        friend class Swapchain;
        friend class RenderPass;
    };

    // @brief Creation information for an image view
    struct ImageViewCreateInfo {
        Image& image;
        ImageViewType type;

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
        data::Reference<Image> image_;

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