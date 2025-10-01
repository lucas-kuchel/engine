#include <renderer/resources/image.hpp>
#include <renderer/resources/pass.hpp>

#include <renderer/device.hpp>

namespace renderer {
    RenderPass::RenderPass(const RenderPassCreateInfo& createInfo)
        : device_(createInfo.device) {
        std::vector<VkAttachmentDescription> attachments;
        std::vector<VkAttachmentReference> colourAttachmentReferences;

        for (std::size_t i = 0; i < createInfo.colourAttachments.size(); ++i) {
            const auto& colourAttachment = createInfo.colourAttachments[i];

            VkAttachmentDescription attachmentDescription = {
                .flags = 0,
                .format = Image::mapFormat(colourAttachment.format),
                .samples = static_cast<VkSampleCountFlagBits>(colourAttachment.sampleCount),
                .loadOp = static_cast<VkAttachmentLoadOp>(colourAttachment.loadOperation),
                .storeOp = static_cast<VkAttachmentStoreOp>(colourAttachment.storeOperation),
                .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
            };

            attachments.push_back(attachmentDescription);

            VkAttachmentReference reference = {
                .attachment = static_cast<std::uint32_t>(i),
                .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            };

            colourAttachmentReferences.push_back(reference);
        }

        VkAttachmentReference depthStencilReference;
        VkAttachmentReference* depthStencilReferencePtr = nullptr;

        VkSubpassDescription subpass = {
            .flags = 0,
            .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
            .inputAttachmentCount = 0,
            .pInputAttachments = nullptr,
            .colorAttachmentCount = static_cast<std::uint32_t>(colourAttachmentReferences.size()),
            .pColorAttachments = colourAttachmentReferences.data(),
            .pResolveAttachments = nullptr,
            .pDepthStencilAttachment = nullptr,
            .preserveAttachmentCount = 0,
            .pPreserveAttachments = nullptr,
        };

        if (createInfo.depthAttachment && createInfo.stencilAttachment) {
            const FrameAttachmentInfo& depthAttachment = createInfo.depthAttachment.get();
            const FrameAttachmentInfo& stencilAttachment = createInfo.stencilAttachment.get();

            if (depthAttachment.format != stencilAttachment.format || depthAttachment.sampleCount != stencilAttachment.sampleCount) {
                throw std::runtime_error("Construction failed: renderer::RenderPass: Mismatched depth and stencil attachments");
            }

            VkAttachmentDescription depthStencilDescription = {
                .flags = 0,
                .format = Image::mapFormat(stencilAttachment.format),
                .samples = static_cast<VkSampleCountFlagBits>(depthAttachment.sampleCount),
                .loadOp = static_cast<VkAttachmentLoadOp>(depthAttachment.loadOperation),
                .storeOp = static_cast<VkAttachmentStoreOp>(depthAttachment.storeOperation),
                .stencilLoadOp = static_cast<VkAttachmentLoadOp>(stencilAttachment.loadOperation),
                .stencilStoreOp = static_cast<VkAttachmentStoreOp>(stencilAttachment.storeOperation),
                .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
            };

            attachments.push_back(depthStencilDescription);

            depthStencilReference = {
                .attachment = static_cast<std::uint32_t>(attachments.size() - 1),
                .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
            };

            depthStencilReferencePtr = &depthStencilReference;
        }
        else if (createInfo.depthAttachment) {
            const FrameAttachmentInfo& depthAttachment = createInfo.depthAttachment.get();

            VkAttachmentDescription depthStencilDescription = {
                .flags = 0,
                .format = Image::mapFormat(depthAttachment.format),
                .samples = static_cast<VkSampleCountFlagBits>(depthAttachment.sampleCount),
                .loadOp = static_cast<VkAttachmentLoadOp>(depthAttachment.loadOperation),
                .storeOp = static_cast<VkAttachmentStoreOp>(depthAttachment.storeOperation),
                .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                .finalLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
            };

            attachments.push_back(depthStencilDescription);

            depthStencilReference = {
                .attachment = static_cast<std::uint32_t>(attachments.size() - 1),
                .layout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
            };

            depthStencilReferencePtr = &depthStencilReference;
        }
        else if (createInfo.stencilAttachment) {
            const FrameAttachmentInfo& stencilAttachment = createInfo.stencilAttachment.get();

            VkAttachmentDescription depthStencilDescription = {
                .flags = 0,
                .format = Image::mapFormat(stencilAttachment.format),
                .samples = static_cast<VkSampleCountFlagBits>(stencilAttachment.sampleCount),
                .loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                .stencilLoadOp = static_cast<VkAttachmentLoadOp>(stencilAttachment.loadOperation),
                .stencilStoreOp = static_cast<VkAttachmentStoreOp>(stencilAttachment.storeOperation),
                .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                .finalLayout = VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL,
            };

            attachments.push_back(depthStencilDescription);

            depthStencilReference = {
                .attachment = static_cast<std::uint32_t>(attachments.size() - 1),
                .layout = VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL,
            };

            depthStencilReferencePtr = &depthStencilReference;
        }

        subpass.pDepthStencilAttachment = depthStencilReferencePtr;

        VkRenderPassCreateInfo renderPassCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .attachmentCount = static_cast<uint32_t>(attachments.size()),
            .pAttachments = attachments.data(),
            .subpassCount = 1,
            .pSubpasses = &subpass,
            .dependencyCount = 0,
            .pDependencies = nullptr,
        };

        if (vkCreateRenderPass(device_->getVkDevice(), &renderPassCreateInfo, nullptr, &renderPass_) != VK_SUCCESS) {
            throw std::runtime_error("Construction failed: renderer::RenderPass: Failed to create render pass");
        }
    }

    RenderPass::~RenderPass() {
        if (renderPass_ != VK_NULL_HANDLE) {
            vkDestroyRenderPass(device_->getVkDevice(), renderPass_, nullptr);
        }
    }

    VkRenderPass& RenderPass::getVkRenderPass() {
        return renderPass_;
    }

    const VkRenderPass& RenderPass::getVkRenderPass() const {
        return renderPass_;
    }
}