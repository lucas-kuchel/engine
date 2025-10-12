#include <renderer/device.hpp>
#include <renderer/image.hpp>
#include <renderer/render_pass.hpp>

namespace renderer {
    RenderPass RenderPass::create(const RenderPassCreateInfo& createInfo) {
        RenderPass renderPass;

        std::vector<VkAttachmentDescription> attachments(createInfo.colourAttachments.size() + createInfo.depthStencilAttachments.size());
        std::vector<VkSubpassDescription> subpasses(createInfo.subpasses.size());
        std::vector<VkSubpassDependency> dependencies(createInfo.subpassDependencies.size());

        const std::uint32_t colourBaseIndex = 0;
        const std::uint32_t depthBaseIndex = static_cast<std::uint32_t>(createInfo.colourAttachments.size());

        for (std::size_t i = 0; i < createInfo.colourAttachments.size(); i++) {
            struct FlagMap {
                ImageLayout layout;
                VkImageLayout vkLayout;
            };

            constexpr FlagMap flagMapping[] = {
                {ImageLayout::UNDEFINED, VK_IMAGE_LAYOUT_UNDEFINED},
                {ImageLayout::PREINITIALIZED, VK_IMAGE_LAYOUT_PREINITIALIZED},
                {ImageLayout::COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
                {ImageLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL},
                {ImageLayout::SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL},
                {ImageLayout::TRANSFER_SOURCE_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL},
                {ImageLayout::TRANSFER_DESTINATION_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL},
                {ImageLayout::GENERAL, VK_IMAGE_LAYOUT_GENERAL},
                {ImageLayout::PRESENT_SOURCE, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR},
            };

            VkImageLayout initial;
            VkImageLayout final;

            for (auto& mapping : flagMapping) {
                if (createInfo.colourAttachments[i].initialLayout == mapping.layout) {
                    initial = mapping.vkLayout;
                }

                if (createInfo.colourAttachments[i].finalLayout == mapping.layout) {
                    final = mapping.vkLayout;
                }
            }

            attachments[i] = {
                .flags = 0,
                .format = Image::mapFormat(createInfo.colourAttachments[i].format, *createInfo.device.instance_),
                .samples = static_cast<VkSampleCountFlagBits>(createInfo.sampleCount),
                .loadOp = static_cast<VkAttachmentLoadOp>(createInfo.colourAttachments[i].operations.load),
                .storeOp = static_cast<VkAttachmentStoreOp>(createInfo.colourAttachments[i].operations.store),
                .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                .initialLayout = initial,
                .finalLayout = final,
            };
        }

        for (std::size_t i = 0; i < attachments.size() - createInfo.colourAttachments.size(); i++) {
            struct FlagMap {
                ImageLayout layout;
                VkImageLayout vkLayout;
            };

            constexpr FlagMap flagMapping[] = {
                {ImageLayout::UNDEFINED, VK_IMAGE_LAYOUT_UNDEFINED},
                {ImageLayout::PREINITIALIZED, VK_IMAGE_LAYOUT_PREINITIALIZED},
                {ImageLayout::COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
                {ImageLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL},
                {ImageLayout::SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL},
                {ImageLayout::TRANSFER_SOURCE_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL},
                {ImageLayout::TRANSFER_DESTINATION_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL},
                {ImageLayout::GENERAL, VK_IMAGE_LAYOUT_GENERAL},
                {ImageLayout::PRESENT_SOURCE, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR},
            };

            VkImageLayout initial;
            VkImageLayout final;

            for (auto& mapping : flagMapping) {
                if (createInfo.depthStencilAttachments[i].initialLayout == mapping.layout) {
                    initial = mapping.vkLayout;
                }

                if (createInfo.depthStencilAttachments[i].finalLayout == mapping.layout) {
                    final = mapping.vkLayout;
                }
            }

            attachments[depthBaseIndex + i] = {
                .flags = 0,
                .format = Image::mapFormat(createInfo.depthStencilAttachments[i].format, *createInfo.device.instance_),
                .samples = static_cast<VkSampleCountFlagBits>(createInfo.sampleCount),
                .loadOp = static_cast<VkAttachmentLoadOp>(createInfo.depthStencilAttachments[i].depthOperations.load),
                .storeOp = static_cast<VkAttachmentStoreOp>(createInfo.depthStencilAttachments[i].depthOperations.store),
                .stencilLoadOp = static_cast<VkAttachmentLoadOp>(createInfo.depthStencilAttachments[i].stencilOperations.load),
                .stencilStoreOp = static_cast<VkAttachmentStoreOp>(createInfo.depthStencilAttachments[i].stencilOperations.store),
                .initialLayout = initial,
                .finalLayout = final,
            };
        }

        std::vector<std::vector<VkAttachmentReference>> inputReferences(createInfo.subpasses.size());
        std::vector<std::vector<VkAttachmentReference>> colourReferences(createInfo.subpasses.size());
        std::vector<VkAttachmentReference> depthReferences(createInfo.subpasses.size());

        for (std::size_t i = 0; i < createInfo.subpasses.size(); i++) {
            const auto& subpassInfo = createInfo.subpasses[i];

            for (auto inputIndex : subpassInfo.colourAttachmentInputIndices) {
                inputReferences[i].push_back({
                    .attachment = static_cast<std::uint32_t>(colourBaseIndex + inputIndex),
                    .layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                });
            }

            for (auto outputIndex : subpassInfo.colourAttachmentOutputIndices) {
                colourReferences[i].push_back({
                    .attachment = static_cast<std::uint32_t>(colourBaseIndex + outputIndex),
                    .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                });
            }

            VkAttachmentReference* depthReferencePointer = nullptr;

            if (subpassInfo.depthStencilIndex.has_value()) {
                auto idx = static_cast<uint32_t>(depthBaseIndex + subpassInfo.depthStencilIndex.value());

                depthReferences[i] = {
                    .attachment = idx,
                    .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                };

                depthReferencePointer = &depthReferences[i];
            }

            subpasses[i] = {
                .flags = 0,
                .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
                .inputAttachmentCount = static_cast<std::uint32_t>(inputReferences[i].size()),
                .pInputAttachments = inputReferences[i].data(),
                .colorAttachmentCount = static_cast<std::uint32_t>(colourReferences[i].size()),
                .pColorAttachments = colourReferences[i].data(),
                .pResolveAttachments = nullptr,
                .pDepthStencilAttachment = depthReferencePointer,
                .preserveAttachmentCount = 0,
                .pPreserveAttachments = nullptr,
            };
        }

        for (std::size_t i = 0; i < dependencies.size(); i++) {
            dependencies[i] = {
                .srcSubpass = createInfo.subpassDependencies[i].subpassSourceIndex.has_value() ? createInfo.subpassDependencies[i].subpassSourceIndex.value() : VK_SUBPASS_EXTERNAL,
                .dstSubpass = createInfo.subpassDependencies[i].subpassDestinationIndex.has_value() ? createInfo.subpassDependencies[i].subpassDestinationIndex.value() : VK_SUBPASS_EXTERNAL,
                .srcStageMask = PipelineStageFlags::mapFrom(createInfo.subpassDependencies[i].stageSourceFlags),
                .dstStageMask = PipelineStageFlags::mapFrom(createInfo.subpassDependencies[i].stageDestinationFlags),
                .srcAccessMask = AccessFlags::mapFrom(createInfo.subpassDependencies[i].accessSourceFlags),
                .dstAccessMask = AccessFlags::mapFrom(createInfo.subpassDependencies[i].accessDestinationFlags),
                .dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT,
            };
        }

        VkRenderPassCreateInfo renderPassCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .attachmentCount = static_cast<uint32_t>(attachments.size()),
            .pAttachments = attachments.data(),
            .subpassCount = static_cast<std::uint32_t>(subpasses.size()),
            .pSubpasses = subpasses.data(),
            .dependencyCount = static_cast<std::uint32_t>(dependencies.size()),
            .pDependencies = dependencies.data(),
        };

        if (vkCreateRenderPass(createInfo.device.device_, &renderPassCreateInfo, nullptr, &renderPass.renderPass_) != VK_SUCCESS) {
            renderPass.renderPass_ = nullptr;
        }
        else {
            renderPass.device_ = &createInfo.device;
        }

        return renderPass;
    }

    void RenderPass::destroy(RenderPass& renderPass) {
        if (renderPass.renderPass_) {
            vkDestroyRenderPass(renderPass.device_->device_, renderPass.renderPass_, nullptr);
        }
    }
}