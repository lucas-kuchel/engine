#include <renderer/resources/framebuffer.hpp>
#include <renderer/resources/image.hpp>
#include <renderer/resources/pass.hpp>

#include <renderer/device.hpp>

namespace renderer {
    Framebuffer::Framebuffer(const FramebufferCreateInfo& createInfo)
        : device_(createInfo.device) {
        std::vector<VkImageView> imageViews;

        imageViews.reserve(createInfo.imageViews.size());

        for (auto& imageView : createInfo.imageViews) {
            imageViews.push_back(imageView->getVkImageView());
        }

        auto& defactoImage = createInfo.imageViews.front();

        VkFramebufferCreateInfo framebufferCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .renderPass = createInfo.renderPass.getVkRenderPass(),
            .attachmentCount = static_cast<std::uint32_t>(createInfo.imageViews.size()),
            .pAttachments = imageViews.data(),
            .width = defactoImage->image().extent().width,
            .height = defactoImage->image().extent().height,
            .layers = 1,
        };

        if (vkCreateFramebuffer(device_->getVkDevice(), &framebufferCreateInfo, nullptr, &framebuffer_) != VK_SUCCESS) {
            throw std::runtime_error("Construction failed: renderer::Framebuffer: Failed to create framebuffer");
        }
    }

    Framebuffer::~Framebuffer() {
        if (framebuffer_ != VK_NULL_HANDLE) {
            vkDestroyFramebuffer(device_->getVkDevice(), framebuffer_, nullptr);
        }
    }

    VkFramebuffer& Framebuffer::getVkFramebuffer() {
        return framebuffer_;
    }

    const VkFramebuffer& Framebuffer::getVkFramebuffer() const {
        return framebuffer_;
    }
}