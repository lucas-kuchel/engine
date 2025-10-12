#include <renderer/framebuffer.hpp>
#include <renderer/image.hpp>
#include <renderer/image_view.hpp>
#include <renderer/render_pass.hpp>

#include <renderer/device.hpp>

namespace renderer {
    Framebuffer Framebuffer::create(const FramebufferCreateInfo& createInfo) {
        Framebuffer framebuffer;

        std::vector<VkImageView> imageViews;

        imageViews.reserve(createInfo.imageViews.size());

        for (auto& imageView : createInfo.imageViews) {
            imageViews.push_back(imageView.imageView_);
        }

        auto& defactoImage = createInfo.imageViews.front();

        VkFramebufferCreateInfo framebufferCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .renderPass = createInfo.renderPass.renderPass_,
            .attachmentCount = static_cast<std::uint32_t>(createInfo.imageViews.size()),
            .pAttachments = imageViews.data(),
            .width = defactoImage.image_->extent_.x,
            .height = defactoImage.image_->extent_.y,
            .layers = 1,
        };

        if (vkCreateFramebuffer(createInfo.device.device_, &framebufferCreateInfo, nullptr, &framebuffer.framebuffer_) != VK_SUCCESS) {
            framebuffer.framebuffer_ = nullptr;
        }
        else {
            framebuffer.device_ = &createInfo.device;
        }

        return framebuffer;
    }

    void Framebuffer::destroy(Framebuffer& framebuffer) {
        if (framebuffer.framebuffer_) {
            vkDestroyFramebuffer(framebuffer.device_->device_, framebuffer.framebuffer_, nullptr);

            framebuffer.framebuffer_ = nullptr;
        }
    }
}