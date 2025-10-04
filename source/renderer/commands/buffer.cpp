#include <renderer/commands/buffer.hpp>
#include <renderer/commands/pool.hpp>

#include <renderer/resources/buffer.hpp>

#include <renderer/device.hpp>

namespace renderer {
    CommandBuffer::~CommandBuffer() {
        endRenderPass();
        endCapture();
    }

    void CommandBuffer::endRenderPass() {
        if (rendering_) {
            rendering_ = false;

            vkCmdEndRenderPass(commandBuffer_);
        }
    }

    void CommandBuffer::bindPipeline(Pipeline& pipeline) {
        if (!rendering_) {
            throw std::runtime_error("Call failed: renderer::CommandBuffer::bindPipeline(): Render pass has ended");
        }

        vkCmdBindPipeline(commandBuffer_, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.getVkPipeline());
    }

    void CommandBuffer::nextSubpass() {
        if (!rendering_) {
            throw std::runtime_error("Call failed: renderer::CommandBuffer::nextSubpass(): Render pass has ended");
        }

        vkCmdNextSubpass(commandBuffer_, VK_SUBPASS_CONTENTS_INLINE);
    }

    void CommandBuffer::bindVertexBuffers(const std::vector<data::Reference<Buffer>>& buffers, const std::vector<std::uint64_t>& offsets, std::uint32_t first) {
        if (!rendering_) {
            throw std::runtime_error("Call failed: renderer::CommandBuffer::bindVertexBuffers(): Render pass has ended");
        }

        std::vector<VkBuffer> vulkanBuffers(buffers.size());

        for (std::uint32_t i = 0; i < vulkanBuffers.size(); i++) {
            vulkanBuffers[i] = buffers[i]->getVkBuffer();
        }

        vkCmdBindVertexBuffers(commandBuffer_, first, static_cast<std::uint32_t>(vulkanBuffers.size()), vulkanBuffers.data(), offsets.data());
    }

    void CommandBuffer::bindIndexBuffer(Buffer& buffer, std::uint64_t offset, IndexType indexType) {
        if (!rendering_) {
            throw std::runtime_error("Call failed: renderer::CommandBuffer::bindIndexBuffer(): Render pass has ended");
        }

        VkIndexType type;

        switch (indexType) {
            case IndexType::UINT16:
                type = VK_INDEX_TYPE_UINT16;
                break;

            case IndexType::UINT32:
                type = VK_INDEX_TYPE_UINT32;
                break;
        }

        vkCmdBindIndexBuffer(commandBuffer_, buffer.getVkBuffer(), offset, type);
    }

    void CommandBuffer::setPipelineViewports(const std::vector<renderer::Viewport>& viewports, std::uint32_t offset) {
        if (!rendering_) {
            throw std::runtime_error("Call failed: renderer::CommandBuffer::setPipelineViewports(): Render pass has ended");
        }

        std::vector<VkViewport> vulkanViewports(viewports.size());

        for (std::uint32_t i = 0; i < vulkanViewports.size(); i++) {
            auto& vulkanViewport = vulkanViewports[i];
            auto& viewport = viewports[i];

            vulkanViewport.x = viewport.position.x;
            vulkanViewport.y = viewport.position.y;

            vulkanViewport.width = viewport.extent.width;
            vulkanViewport.height = viewport.extent.height;

            vulkanViewport.minDepth = viewport.depth.min;
            vulkanViewport.maxDepth = viewport.depth.max;
        }

        vkCmdSetViewport(commandBuffer_, offset, static_cast<std::uint32_t>(vulkanViewports.size()), vulkanViewports.data());
    }

    void CommandBuffer::setPipelineScissors(const std::vector<renderer::Scissor>& scissors, std::uint32_t offset) {
        if (!rendering_) {
            throw std::runtime_error("Call failed: renderer::CommandBuffer::setPipelineScissors(): Render pass has ended");
        }

        std::vector<VkRect2D> vulkanScissors(scissors.size());

        for (std::uint32_t i = 0; i < vulkanScissors.size(); i++) {
            auto& vulkanScissor = vulkanScissors[i];
            auto& scissor = scissors[i];

            vulkanScissor.offset.x = scissor.offset.x;
            vulkanScissor.offset.y = scissor.offset.y;

            vulkanScissor.extent.width = scissor.extent.width;
            vulkanScissor.extent.height = scissor.extent.height;
        }

        vkCmdSetScissor(commandBuffer_, offset, static_cast<std::uint32_t>(vulkanScissors.size()), vulkanScissors.data());
    }

    void CommandBuffer::setPipelineLineWidth(float width) {
        if (!rendering_) {
            throw std::runtime_error("Call failed: renderer::CommandBuffer::setPipelineLineWidth(): Render pass has ended");
        }

        vkCmdSetLineWidth(commandBuffer_, width);
    }

    void CommandBuffer::setPipelineDepthBias(float depthBiasConstantFactor, float depthBiasClamp, float depthBiasSlopeFactor) {
        if (!rendering_) {
            throw std::runtime_error("Call failed: renderer::CommandBuffer::setPipelineDepthBias(): Render pass has ended");
        }

        vkCmdSetDepthBias(commandBuffer_, depthBiasConstantFactor, depthBiasClamp, depthBiasSlopeFactor);
    }

    void CommandBuffer::setPipelineBlendConstants(BlendConstants blend) {
        if (!rendering_) {
            throw std::runtime_error("Call failed: renderer::CommandBuffer::setPipelineBlendConstants(): Render pass has ended");
        }

        vkCmdSetBlendConstants(commandBuffer_, &blend.r);
    }

    void CommandBuffer::setPipelineDepthBounds(data::Range<float> range) {
        if (!rendering_) {
            throw std::runtime_error("Call failed: renderer::CommandBuffer::setPipelineDepthBounds(): Render pass has ended");
        }

        vkCmdSetDepthBounds(commandBuffer_, range.min, range.max);
    }

    void CommandBuffer::setPipelineStencilCompareMask(Flags faceFlags, std::uint32_t compareMask) {
        if (!rendering_) {
            throw std::runtime_error("Call failed: renderer::CommandBuffer::setPipelineStencilCompareMask(): Render pass has ended");
        }

        vkCmdSetStencilCompareMask(commandBuffer_, StencilFaceFlags::mapFrom(faceFlags), compareMask);
    }

    void CommandBuffer::setPipelineStencilWriteMask(Flags faceFlags, std::uint32_t writeMask) {
        if (!rendering_) {
            throw std::runtime_error("Call failed: renderer::CommandBuffer::setPipelineStencilWriteMask(): Render pass has ended");
        }

        vkCmdSetStencilWriteMask(commandBuffer_, StencilFaceFlags::mapFrom(faceFlags), writeMask);
    }

    void CommandBuffer::setPipelineStencilReferenceMask(Flags faceFlags, std::uint32_t reference) {
        if (!rendering_) {
            throw std::runtime_error("Call failed: renderer::CommandBuffer::setPipelineStencilReferenceMask(): Render pass has ended");
        }

        vkCmdSetStencilReference(commandBuffer_, StencilFaceFlags::mapFrom(faceFlags), reference);
    }

    void CommandBuffer::pushConstants(PipelineLayout& layout, std::uint32_t stageFlags, std::span<std::uint8_t> data, std::uint32_t offset) {
        if (!rendering_) {
            throw std::runtime_error("Call failed: renderer::CommandBuffer::pushConstants(): Render pass has ended");
        }

        VkShaderStageFlags flags = 0;

        if (stageFlags & DescriptorShaderStageFlags::VERTEX) {
            flags |= VK_SHADER_STAGE_VERTEX_BIT;
        }

        if (stageFlags & DescriptorShaderStageFlags::FRAGMENT) {
            flags |= VK_SHADER_STAGE_FRAGMENT_BIT;
        }

        vkCmdPushConstants(commandBuffer_, layout.getVkPipelineLayout(), flags, offset, static_cast<std::uint32_t>(data.size()), data.data());
    }

    void CommandBuffer::draw(std::uint32_t vertexCount, std::uint32_t instances, std::uint32_t firstVertex, std::uint32_t firstInstance) {
        if (!rendering_) {
            throw std::runtime_error("Call failed: renderer::CommandBuffer::draw(): Render pass has ended");
        }

        vkCmdDraw(commandBuffer_, vertexCount, instances, firstVertex, firstInstance);
    }

    void CommandBuffer::drawIndexed(std::uint32_t indexCount, std::uint32_t instanceCount, std::uint32_t firstIndex, std::uint32_t firstInstance, std::int32_t vertexOffset) {
        if (!rendering_) {
            throw std::runtime_error("Call failed: renderer::CommandBuffer::drawIndexed(): Render pass has ended");
        }

        vkCmdDrawIndexed(commandBuffer_, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
    }

    bool CommandBuffer::rendering() const {
        return rendering_;
    }

    void CommandBuffer::beginRenderPass(RenderPassBeginInfo& beginInfo) {
        rendering_ = true;

        std::uint32_t clearValueCount = static_cast<std::uint32_t>(beginInfo.clearValues.size());

        if (beginInfo.depthClearValue || beginInfo.stencilClearValue) {
            clearValueCount += 1;
        }

        std::vector<VkClearValue> clearValues(clearValueCount);

        for (std::size_t i = 0; i < beginInfo.clearValues.size(); i++) {
            auto& vkClearValue = clearValues[i];
            auto& clearValue = beginInfo.clearValues[i];

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
                .depth = beginInfo.depthClearValue.get(),
                .stencil = beginInfo.stencilClearValue.get(),
            };
        }

        VkRenderPassBeginInfo renderPassBeginInfo = {
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
            .pNext = nullptr,
            .renderPass = beginInfo.renderPass.getVkRenderPass(),
            .framebuffer = beginInfo.framebuffer.getVkFramebuffer(),
            .renderArea = {
                {
                    beginInfo.renderArea.offset.x,
                    beginInfo.renderArea.offset.y,
                },
                {
                    beginInfo.renderArea.extent.width,
                    beginInfo.renderArea.extent.height,
                },
            },
            .clearValueCount = clearValueCount,
            .pClearValues = clearValues.data(),
        };

        vkCmdBeginRenderPass(commandBuffer_, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
    }

    void CommandBuffer::endCapture() {
        if (capturing_ && !rendering_) {
            capturing_ = false;

            if (vkEndCommandBuffer(commandBuffer_) != VK_SUCCESS) {
                throw std::runtime_error("Call failed: renderer::CommandBuffer::end(): Failed to end command buffer capture");
            }
        }
    }

    void CommandBuffer::copyBuffer(Buffer& source, Buffer& destination, const std::vector<BufferCopyRegion>& copyRegions) {
        if (!capturing_) {
            throw std::runtime_error("Call failed: renderer::CommandBuffer::copyBuffer(): Render pass has ended");
        }

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

        vkCmdCopyBuffer(commandBuffer_, source.getVkBuffer(), destination.getVkBuffer(), static_cast<std::uint32_t>(bufferCopies.size()), bufferCopies.data());
    }

    void CommandBuffer::bindDescriptorSets(DeviceOperation operation, PipelineLayout& layout, std::uint32_t firstSet, const std::vector<data::Reference<DescriptorSet>>& sets) {
        if (!capturing_) {
            throw std::runtime_error("Call failed: renderer::CommandBuffer::bindDescriptorSets(): Render pass has ended");
        }

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
            vkSets[i] = sets[i]->getVkDescriptorSet();
        }

        vkCmdBindDescriptorSets(commandBuffer_, point, layout.getVkPipelineLayout(), firstSet, static_cast<std::uint32_t>(vkSets.size()), vkSets.data(), 0, nullptr);
    }

    bool CommandBuffer::capturing() const {
        return capturing_;
    }

    void CommandBuffer::beginCapture() {
        capturing_ = true;

        VkCommandBufferBeginInfo commandBufferBeginInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .pNext = nullptr,
            .flags = 0,
            .pInheritanceInfo = nullptr,
        };

        if (vkBeginCommandBuffer(commandBuffer_, &commandBufferBeginInfo) != VK_SUCCESS) {
            throw std::runtime_error("Construction failed: renderer::CommandBuffer: Failed to start command buffer capture");
        }
    }

    void CommandBuffer::reset() {
        vkResetCommandBuffer(commandBuffer_, 0);
    }

    VkCommandBuffer& CommandBuffer::getVkCommandBuffer() {
        return commandBuffer_;
    }

    const VkCommandBuffer& CommandBuffer::getVkCommandBuffer() const {
        return commandBuffer_;
    }

    CommandBuffer::CommandBuffer(CommandPool& comandPool)
        : commandPool_(comandPool) {
    }
}