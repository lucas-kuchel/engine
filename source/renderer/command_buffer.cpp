#include <renderer/buffer.hpp>
#include <renderer/command_buffer.hpp>
#include <renderer/command_pool.hpp>
#include <renderer/framebuffer.hpp>
#include <renderer/image.hpp>
#include <renderer/image_view.hpp>
#include <renderer/pipeline.hpp>
#include <renderer/render_pass.hpp>

#include <renderer/device.hpp>

namespace renderer {
    void CommandBuffer::reset(CommandBuffer& commandBuffer) {
        vkResetCommandBuffer(commandBuffer.commandBuffer_, 0);
    }

    bool CommandBuffer::beginCapture(CommandBuffer& commandBuffer) {
        commandBuffer.capturing_ = true;

        VkCommandBufferBeginInfo commandBufferBeginInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .pNext = nullptr,
            .flags = 0,
            .pInheritanceInfo = nullptr,
        };

        return vkBeginCommandBuffer(commandBuffer.commandBuffer_, &commandBufferBeginInfo) == VK_SUCCESS;
    }

    void CommandBuffer::beginRenderPass(CommandBuffer& commandBuffer, RenderPassBeginInfo& beginInfo) {
        commandBuffer.rendering_ = true;

        std::uint32_t clearValueCount = static_cast<std::uint32_t>(beginInfo.colourClearValues.size());

        if (beginInfo.depthClearValue || beginInfo.stencilClearValue) {
            clearValueCount += 1;
        }

        std::vector<VkClearValue> clearValues(clearValueCount);

        for (std::size_t i = 0; i < beginInfo.colourClearValues.size(); i++) {
            auto& vkClearValue = clearValues[i];
            auto& clearValue = beginInfo.colourClearValues[i];

            vkClearValue.color = {
                {
                    clearValue.r,
                    clearValue.g,
                    clearValue.b,
                    clearValue.a,
                },
            };
        }

        if (beginInfo.depthClearValue || beginInfo.stencilClearValue) {
            std::size_t index = clearValues.size() - 1;

            auto& vkClearValue = clearValues[index];

            vkClearValue.depthStencil = {
                .depth = beginInfo.depthClearValue.value(),
                .stencil = beginInfo.stencilClearValue.value(),
            };
        }

        VkRenderPassBeginInfo renderPassBeginInfo = {
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
            .pNext = nullptr,
            .renderPass = beginInfo.renderPass.renderPass_,
            .framebuffer = beginInfo.framebuffer.framebuffer_,
            .renderArea = {
                {
                    beginInfo.region.position.x,
                    beginInfo.region.position.y,
                },
                {
                    beginInfo.region.extent.x,
                    beginInfo.region.extent.y,
                },
            },
            .clearValueCount = clearValueCount,
            .pClearValues = clearValues.data(),
        };

        vkCmdBeginRenderPass(commandBuffer.commandBuffer_, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
    }

    bool CommandBuffer::endCapture(CommandBuffer& commandBuffer) {
        if (!commandBuffer.capturing_) {
            return false;
        }

        commandBuffer.capturing_ = false;

        return vkEndCommandBuffer(commandBuffer.commandBuffer_) == VK_SUCCESS;
    }

    void CommandBuffer::endRenderPass(CommandBuffer& commandBuffer) {
        if (!commandBuffer.rendering_) {
            return;
        }

        commandBuffer.rendering_ = false;

        vkCmdEndRenderPass(commandBuffer.commandBuffer_);
    }

    void CommandBuffer::copyBuffer(CommandBuffer& commandBuffer, Buffer& source, Buffer& destination, const std::vector<BufferCopyRegion>& copyRegions) {
        std::vector<VkBufferCopy> bufferCopies(copyRegions.size());

        for (std::size_t i = 0; i < bufferCopies.size(); i++) {
            auto& bufferCopy = bufferCopies[i];
            auto& copyRegion = copyRegions[i];

            bufferCopy = {
                .srcOffset = copyRegion.sourceOffsetBytes,
                .dstOffset = copyRegion.destinationOffsetBytes,
                .size = copyRegion.sizeBytes,
            };
        }

        vkCmdCopyBuffer(commandBuffer.commandBuffer_, source.buffer_, destination.buffer_, static_cast<std::uint32_t>(bufferCopies.size()), bufferCopies.data());
    }

    void CommandBuffer::copyBufferToImage(CommandBuffer& commandBuffer, Buffer& source, Image& destination, ImageLayout imageLayout, const std::vector<BufferImageCopyRegion>& copyRegions) {
        std::vector<VkBufferImageCopy> copies(copyRegions.size());

        for (std::size_t i = 0; i < copies.size(); i++) {
            auto& copy = copies[i];
            auto& copyRegion = copyRegions[i];

            copy = {
                .bufferOffset = copyRegion.bufferOffset,
                .bufferRowLength = copyRegion.bufferRowLength,
                .bufferImageHeight = copyRegion.bufferImageHeight,
                .imageSubresource = {
                    .aspectMask = ImageAspectFlags::mapFrom(copyRegion.imageAspectMask),
                    .mipLevel = copyRegion.mipLevel,
                    .baseArrayLayer = copyRegion.baseArrayLayer,
                    .layerCount = copyRegion.arrayLayerCount,
                },
                .imageOffset = {
                    copyRegion.imageOffset.x,
                    copyRegion.imageOffset.y,
                    copyRegion.imageOffset.z,
                },
                .imageExtent = {
                    copyRegion.imageExtent.x,
                    copyRegion.imageExtent.y,
                    copyRegion.imageExtent.z,
                },
            };
        }

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

        VkImageLayout layout;

        for (auto& mapping : flagMapping) {
            if (imageLayout == mapping.layout) {
                layout = mapping.vkLayout;
                break;
            }
        }

        vkCmdCopyBufferToImage(commandBuffer.commandBuffer_, source.buffer_, destination.image_, layout, static_cast<std::uint32_t>(copies.size()), copies.data());
    }

    void CommandBuffer::nextSubpass(CommandBuffer& commandBuffer) {
        vkCmdNextSubpass(commandBuffer.commandBuffer_, VK_SUBPASS_CONTENTS_INLINE);
    }

    void CommandBuffer::pipelineBarrier(CommandBuffer& commandBuffer, Flags sourcePipelineStage, Flags destinationPipelineStage, const std::vector<ImageMemoryBarrier>& memoryBarriers) {
        std::vector<VkImageMemoryBarrier> barriers(memoryBarriers.size());

        for (std::size_t i = 0; i < barriers.size(); i++) {

            VkImageLayout oldLayout;
            VkImageLayout newLayout;

            switch (memoryBarriers[i].oldLayout) {
                case ImageLayout::UNDEFINED:
                    oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                    break;

                case ImageLayout::GENERAL:
                    oldLayout = VK_IMAGE_LAYOUT_GENERAL;
                    break;

                case ImageLayout::COLOR_ATTACHMENT_OPTIMAL:
                    oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                    break;

                case ImageLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
                    oldLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
                    break;

                case ImageLayout::SHADER_READ_ONLY_OPTIMAL:
                    oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                    break;

                case ImageLayout::TRANSFER_SOURCE_OPTIMAL:
                    oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
                    break;

                case ImageLayout::TRANSFER_DESTINATION_OPTIMAL:
                    oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
                    break;

                case ImageLayout::PRESENT_SOURCE:
                    oldLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
                    break;

                case ImageLayout::PREINITIALIZED:
                    oldLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
                    break;
            }

            switch (memoryBarriers[i].newLayout) {
                case ImageLayout::UNDEFINED:
                    newLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                    break;

                case ImageLayout::GENERAL:
                    newLayout = VK_IMAGE_LAYOUT_GENERAL;
                    break;

                case ImageLayout::COLOR_ATTACHMENT_OPTIMAL:
                    newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                    break;

                case ImageLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
                    newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
                    break;

                case ImageLayout::SHADER_READ_ONLY_OPTIMAL:
                    newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                    break;

                case ImageLayout::TRANSFER_SOURCE_OPTIMAL:
                    newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
                    break;

                case ImageLayout::TRANSFER_DESTINATION_OPTIMAL:
                    newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
                    break;

                case ImageLayout::PRESENT_SOURCE:
                    newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
                    break;

                case ImageLayout::PREINITIALIZED:
                    newLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
                    break;
            }

            barriers[i] = {
                .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                .pNext = nullptr,
                .srcAccessMask = AccessFlags::mapFrom(memoryBarriers[i].sourceAccessFlags),
                .dstAccessMask = AccessFlags::mapFrom(memoryBarriers[i].destinationAccessFlags),
                .oldLayout = oldLayout,
                .newLayout = newLayout,
                .srcQueueFamilyIndex = memoryBarriers[i].sourceQueue != nullptr ? memoryBarriers[i].sourceQueue->familyIndex_ : VK_QUEUE_FAMILY_IGNORED,
                .dstQueueFamilyIndex = memoryBarriers[i].destinationQueue != nullptr ? memoryBarriers[i].destinationQueue->familyIndex_ : VK_QUEUE_FAMILY_IGNORED,
                .image = memoryBarriers[i].image.image_,
                .subresourceRange = {
                    .aspectMask = memoryBarriers[i].aspectMask,
                    .baseMipLevel = memoryBarriers[i].baseMipLevel,
                    .levelCount = memoryBarriers[i].mipLevelCount,
                    .baseArrayLayer = memoryBarriers[i].baseArrayLayer,
                    .layerCount = memoryBarriers[i].arrayLayerCount,
                },
            };
        }

        vkCmdPipelineBarrier(commandBuffer.commandBuffer_, PipelineStageFlags::mapFrom(sourcePipelineStage), PipelineStageFlags::mapFrom(destinationPipelineStage), 0, 0, nullptr, 0, nullptr, static_cast<std::uint32_t>(barriers.size()), barriers.data());
    }

    void CommandBuffer::bindDescriptorSets(CommandBuffer& commandBuffer, DeviceOperation operation, PipelineLayout& layout, std::uint32_t firstSet, const std::vector<DescriptorSet>& sets) {
        VkPipelineBindPoint point;

        switch (operation) {
            case renderer::DeviceOperation::GRAPHICS:
                point = VK_PIPELINE_BIND_POINT_GRAPHICS;
                break;

            case renderer::DeviceOperation::COMPUTE:
                point = VK_PIPELINE_BIND_POINT_COMPUTE;
                break;
        }

        std::vector<VkDescriptorSet> vkSets(sets.size());

        for (std::size_t i = 0; i < vkSets.size(); i++) {
            vkSets[i] = sets[i].descriptorSet_;
        }

        vkCmdBindDescriptorSets(commandBuffer.commandBuffer_, point, layout.pipelineLayout_, firstSet, static_cast<std::uint32_t>(vkSets.size()), vkSets.data(), 0, nullptr);
    }

    void CommandBuffer::bindPipeline(CommandBuffer& commandBuffer, Pipeline& pipeline) {
        vkCmdBindPipeline(commandBuffer.commandBuffer_, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.pipeline_);
    }

    void CommandBuffer::bindVertexBuffers(CommandBuffer& commandBuffer, const std::vector<Buffer>& buffers, const std::vector<std::size_t>& offsets, std::uint32_t first) {
        std::vector<VkBuffer> vulkanBuffers(buffers.size());

        for (std::uint32_t i = 0; i < vulkanBuffers.size(); i++) {
            vulkanBuffers[i] = buffers[i].buffer_;
        }

        vkCmdBindVertexBuffers(commandBuffer.commandBuffer_, first, static_cast<std::uint32_t>(vulkanBuffers.size()), vulkanBuffers.data(), offsets.data());
    }

    void CommandBuffer::bindIndexBuffer(CommandBuffer& commandBuffer, Buffer& buffer, std::size_t offset, IndexType indexType) {
        VkIndexType type;

        switch (indexType) {
            case IndexType::UINT16:
                type = VK_INDEX_TYPE_UINT16;
                break;

            case IndexType::UINT32:
                type = VK_INDEX_TYPE_UINT32;
                break;
        }

        vkCmdBindIndexBuffer(commandBuffer.commandBuffer_, buffer.buffer_, offset, type);
    }

    void CommandBuffer::setPipelineViewports(CommandBuffer& commandBuffer, const std::vector<renderer::Viewport>& viewports, std::uint32_t offset) {
        std::vector<VkViewport> vulkanViewports(viewports.size());

        for (std::uint32_t i = 0; i < vulkanViewports.size(); i++) {
            auto& vulkanViewport = vulkanViewports[i];
            auto& viewport = viewports[i];

            vulkanViewport.x = viewport.position.x;
            vulkanViewport.y = viewport.position.y;

            vulkanViewport.width = viewport.extent.x;
            vulkanViewport.height = viewport.extent.y;

            vulkanViewport.minDepth = viewport.minDepth;
            vulkanViewport.maxDepth = viewport.maxDepth;
        }

        vkCmdSetViewport(commandBuffer.commandBuffer_, offset, static_cast<std::uint32_t>(vulkanViewports.size()), vulkanViewports.data());
    }

    void CommandBuffer::setPipelineScissors(CommandBuffer& commandBuffer, const std::vector<renderer::Scissor>& scissors, std::uint32_t offset) {
        std::vector<VkRect2D> vulkanScissors(scissors.size());

        for (std::uint32_t i = 0; i < vulkanScissors.size(); i++) {
            auto& vulkanScissor = vulkanScissors[i];
            auto& scissor = scissors[i];

            vulkanScissor.offset.x = scissor.offset.x;
            vulkanScissor.offset.y = scissor.offset.y;

            vulkanScissor.extent.width = scissor.extent.x;
            vulkanScissor.extent.height = scissor.extent.y;
        }

        vkCmdSetScissor(commandBuffer.commandBuffer_, offset, static_cast<std::uint32_t>(vulkanScissors.size()), vulkanScissors.data());
    }

    void CommandBuffer::setPipelineLineWidth(CommandBuffer& commandBuffer, float width) {
        vkCmdSetLineWidth(commandBuffer.commandBuffer_, width);
    }

    void CommandBuffer::setPipelineDepthBias(CommandBuffer& commandBuffer, float depthBiasConstantFactor, float depthBiasClamp, float depthBiasSlopeFactor) {
        vkCmdSetDepthBias(commandBuffer.commandBuffer_, depthBiasConstantFactor, depthBiasClamp, depthBiasSlopeFactor);
    }

    void CommandBuffer::setPipelineBlendConstants(CommandBuffer& commandBuffer, const glm::fvec4& blend) {
        vkCmdSetBlendConstants(commandBuffer.commandBuffer_, &blend.r);
    }

    void CommandBuffer::setPipelineDepthBounds(CommandBuffer& commandBuffer, float min, float max) {
        vkCmdSetDepthBounds(commandBuffer.commandBuffer_, min, max);
    }

    void CommandBuffer::setPipelineStencilCompareMask(CommandBuffer& commandBuffer, Flags faceFlags, std::uint32_t compareMask) {
        vkCmdSetStencilCompareMask(commandBuffer.commandBuffer_, StencilFaceFlags::mapFrom(faceFlags), compareMask);
    }

    void CommandBuffer::setPipelineStencilWriteMask(CommandBuffer& commandBuffer, Flags faceFlags, std::uint32_t writeMask) {
        vkCmdSetStencilWriteMask(commandBuffer.commandBuffer_, StencilFaceFlags::mapFrom(faceFlags), writeMask);
    }

    void CommandBuffer::setPipelineStencilReferenceMask(CommandBuffer& commandBuffer, Flags faceFlags, std::uint32_t reference) {
        vkCmdSetStencilReference(commandBuffer.commandBuffer_, StencilFaceFlags::mapFrom(faceFlags), reference);
    }

    void CommandBuffer::pushConstants(CommandBuffer& commandBuffer, PipelineLayout& layout, std::uint32_t stageFlags, std::span<std::uint8_t> data, std::uint32_t offset) {
        VkShaderStageFlags flags = 0;

        if (stageFlags & DescriptorShaderStageFlags::VERTEX) {
            flags |= VK_SHADER_STAGE_VERTEX_BIT;
        }

        if (stageFlags & DescriptorShaderStageFlags::FRAGMENT) {
            flags |= VK_SHADER_STAGE_FRAGMENT_BIT;
        }

        vkCmdPushConstants(commandBuffer.commandBuffer_, layout.pipelineLayout_, flags, offset, static_cast<std::uint32_t>(data.size()), data.data());
    }

    void CommandBuffer::draw(CommandBuffer& commandBuffer, std::uint32_t vertexCount, std::uint32_t instances, std::uint32_t firstVertex, std::uint32_t firstInstance) {
        vkCmdDraw(commandBuffer.commandBuffer_, vertexCount, instances, firstVertex, firstInstance);
    }

    void CommandBuffer::drawIndexed(CommandBuffer& commandBuffer, std::uint32_t indexCount, std::uint32_t instanceCount, std::uint32_t firstIndex, std::uint32_t firstInstance, std::int32_t vertexOffset) {
        vkCmdDrawIndexed(commandBuffer.commandBuffer_, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
    }

    bool CommandBuffer::capturing(CommandBuffer& commandBuffer) {
        return commandBuffer.capturing_;
    }

    bool CommandBuffer::rendering(CommandBuffer& commandBuffer) {
        return commandBuffer.rendering_;
    }
}