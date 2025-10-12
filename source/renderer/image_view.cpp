#include <renderer/device.hpp>
#include <renderer/image.hpp>
#include <renderer/image_view.hpp>

namespace renderer {
    ImageView ImageView::create(const ImageViewCreateInfo& createInfo) {
        ImageView imageView;

        VkImageViewCreateInfo viewCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .image = createInfo.image.image_,
            .viewType = ImageView::mapType(createInfo.type),
            .format = createInfo.image.format_,
            .components = {},
            .subresourceRange = {
                .aspectMask = ImageAspectFlags::mapFrom(createInfo.aspectFlags),
                .baseMipLevel = createInfo.baseMipLevel,
                .levelCount = createInfo.levelCount,
                .baseArrayLayer = createInfo.baseArrayLayer,
                .layerCount = createInfo.layerCount,
            },
        };

        if (vkCreateImageView(createInfo.image.device_->device_, &viewCreateInfo, nullptr, &imageView.imageView_) != VK_SUCCESS) {
            imageView.imageView_ = nullptr;
        }
        else {
            imageView.imageViewType_ = mapType(createInfo.type);
            imageView.image_ = &createInfo.image;
            imageView.baseArrayLayer_ = createInfo.baseArrayLayer;
            imageView.baseMipLevel_ = createInfo.baseMipLevel;
            imageView.layerCount_ = createInfo.layerCount;
            imageView.levelCount_ = createInfo.levelCount;
        }

        return imageView;
    }

    void ImageView::destroy(ImageView& imageView) {
        if (imageView.imageView_) {
            vkDestroyImageView(imageView.image_->device_->device_, imageView.imageView_, nullptr);

            imageView.imageView_ = nullptr;
        }
    }

    const Image& ImageView::getImage(ImageView& imageView) {
        return *imageView.image_;
    }

    ImageViewType ImageView::getType(ImageView& imageView) {
        return reverseMapType(imageView.imageViewType_);
    }

    std::uint32_t ImageView::getBaseMipLevel(ImageView& imageView) {
        return imageView.baseMipLevel_;
    }

    std::uint32_t ImageView::getLevelCount(ImageView& imageView) {
        return imageView.levelCount_;
    }

    std::uint32_t ImageView::getBaseArrayLayer(ImageView& imageView) {
        return imageView.baseArrayLayer_;
    }

    std::uint32_t ImageView::getLayerCount(ImageView& imageView) {
        return imageView.layerCount_;
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
                return VK_IMAGE_VIEW_TYPE_MAX_ENUM;
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
                return ImageViewType::IMAGE_1D;
        }
    }
}