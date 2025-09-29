#pragma once

#include <renderer/commands/buffer.hpp>
#include <renderer/commands/sync.hpp>

#include <data/extent.hpp>

#include <cstdint>

#include <vulkan/vulkan.h>

namespace renderer {
    // @brief Image dimensionality
    enum class ImageType {
        IMAGE_1D,
        IMAGE_2D,
        IMAGE_3D,
    };

    // @brief Image data format
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
        S8_UINT,
        D32_SFLOAT_S8_UINT,
    };

    // @brief Image usage flags
    struct ImageUsageFlags {
        enum {
            NONE = 0,
            TRANSFER_SOURCE = 1 << 0,
            TRANSFER_DESTINATION = 1 << 1,
            SAMPLED = 1 << 2,
            STORAGE = 1 << 3,
            COLOR_ATTACHMENT = 1 << 4,
            DEPTH_ATTACHMENT = 1 << 5,
            STENCIL_ATTACHMENT = 1 << 6,
        };
    };

    // @brief Creation information for the image
    struct ImageCreateInfo {
        Device& device;

        ImageType type;
        ImageFormat format;

        data::Extent3D<std::uint32_t> extent;

        std::uint32_t usageFlags;
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

        [[nodiscard]] ImageFormat getFormat() const;
        [[nodiscard]] ImageType getType() const;

        [[nodiscard]] data::Extent3D<std::uint32_t> getExtent() const;

        [[nodiscard]] std::uint32_t getSampleCount() const;
        [[nodiscard]] std::uint32_t getMipLevels() const;
        [[nodiscard]] std::uint32_t getArrayLayers() const;
        [[nodiscard]] std::uint32_t getUsageFlags() const;

        [[nodiscard]] VkImage& getVkImage();
        [[nodiscard]] VkDeviceMemory& getVkDeviceMemory();

        [[nodiscard]] const VkImage& getVkImage() const;
        [[nodiscard]] const VkDeviceMemory& getVkDeviceMemory() const;

        [[nodiscard]] VkImageType getVkImageType() const;
        [[nodiscard]] VkFormat getVkFormat() const;
        [[nodiscard]] VkImageUsageFlags getVkImageUsageFlags() const;

    private:
        Image(Device& device);

        data::Reference<Device> device_;

        data::Extent3D<std::uint32_t> extent_;

        std::uint32_t sampleCount_;
        std::uint32_t mipLevels_;
        std::uint32_t arrayLayers_;

        VkImage image_ = VK_NULL_HANDLE;
        VkDeviceMemory deviceMemory_ = VK_NULL_HANDLE;
        VkImageType type_;
        VkFormat format_;
        VkImageUsageFlags usageFlags_;

        static VkFormat mapFormat(ImageFormat format);
        static ImageFormat reverseMapFormat(VkFormat format);

        static VkImageType mapType(ImageType type);
        static ImageType reverseMapType(VkImageType type);

        static VkImageUsageFlags mapFlags(std::uint32_t usageFlags);
        static std::uint32_t reverseMapFlags(VkImageUsageFlags usageFlags);

        friend class Swapchain;
        friend class RenderPass;
    };

    // @brief Dimensionality of an image view
    enum class ImageViewType {
        IMAGE_1D,
        IMAGE_2D,
        IMAGE_3D,
    };

    // @brief Creation information for an image view
    struct ImageViewCreateInfo {
        Device& device;
        Image& image;
        ImageViewType type;

        std::uint32_t baseMipLevel = 0;
        std::uint32_t levelCount = 1;
        std::uint32_t baseArrayLayer = 0;
        std::uint32_t layerCount = 1;
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

        [[nodiscard]] const Image& getImage() const;

        [[nodiscard]] ImageViewType getImageViewType() const;

        [[nodiscard]] VkImageView& getVkImageView();
        [[nodiscard]] const VkImageView& getVkImageView() const;

        [[nodiscard]] VkImageViewType getVkImageViewType() const;

    private:
        data::Reference<Image> image_;
        data::Reference<Device> device_;

        VkImageView imageView_ = VK_NULL_HANDLE;
        VkImageViewType type_;

        static VkImageViewType mapType(ImageViewType type);
        static ImageViewType reverseMapType(VkImageViewType type);
    };
}