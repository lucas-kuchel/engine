#pragma once

#include <renderer/configuration.hpp>

namespace renderer {
    class Image;
    class Queue;

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

    class ImageView {
    public:
        static ImageView create(const ImageViewCreateInfo& createInfo);
        static void destroy(ImageView& imageView);

        static const Image& getImage(ImageView& imageView);
        static ImageViewType getType(ImageView& imageView);
        static std::uint32_t getBaseMipLevel(ImageView& imageView);
        static std::uint32_t getLevelCount(ImageView& imageView);
        static std::uint32_t getBaseArrayLayer(ImageView& imageView);
        static std::uint32_t getLayerCount(ImageView& imageView);

    private:
        Image* image_ = nullptr;
        VkImageView imageView_ = nullptr;

        VkImageViewType imageViewType_ = VK_IMAGE_VIEW_TYPE_MAX_ENUM;

        std::uint32_t baseMipLevel_ = 0;
        std::uint32_t levelCount_ = 0;
        std::uint32_t baseArrayLayer_ = 0;
        std::uint32_t layerCount_ = 0;

        static VkImageViewType mapType(ImageViewType type);
        static ImageViewType reverseMapType(VkImageViewType type);

        friend class Framebuffer;
        friend class DescriptorPool;
    };

    struct ImageMemoryBarrier {
        Image& image;

        Queue* sourceQueue;
        Queue* destinationQueue;

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
}