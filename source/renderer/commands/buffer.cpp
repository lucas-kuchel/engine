#include <renderer/commands/buffer.hpp>
#include <renderer/commands/pool.hpp>

#include <renderer/resources/buffer.hpp>

#include <renderer/device.hpp>

namespace renderer {
    CommandBufferRenderPassPeriod::~CommandBufferRenderPassPeriod() {
        end();
    }

    void CommandBufferRenderPassPeriod::end() {
        if (rendering_.get()) {
            rendering_.get() = false;

            vkCmdEndRenderPass(commandBuffer_->getVkCommandBuffer());
        }
    }

    void CommandBufferRenderPassPeriod::bindPipeline(Pipeline& pipeline) {
        if (!rendering_.get()) {
            throw std::runtime_error("Call failed: renderer::CommandBufferRenderPassPeriod::bindPipeline(): Render pass has ended");
        }

        vkCmdBindPipeline(commandBuffer_->getVkCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.getVkPipeline());
    }

    void CommandBufferRenderPassPeriod::bindVertexBuffers(const std::vector<data::Reference<Buffer>>& buffers, const std::vector<std::uint64_t>& offsets, std::uint32_t first) {
        if (!rendering_.get()) {
            throw std::runtime_error("Call failed: renderer::CommandBufferRenderPassPeriod::bindVertexBuffers(): Render pass has ended");
        }

        std::vector<VkBuffer> vulkanBuffers(buffers.size());

        for (std::uint32_t i = 0; i < vulkanBuffers.size(); i++) {
            vulkanBuffers[i] = buffers[i]->getVkBuffer();
        }

        vkCmdBindVertexBuffers(commandBuffer_->getVkCommandBuffer(), first, static_cast<std::uint32_t>(vulkanBuffers.size()), vulkanBuffers.data(), offsets.data());
    }

    void CommandBufferRenderPassPeriod::bindIndexBuffer(Buffer& buffer, std::uint64_t offset, IndexType indexType) {
        if (!rendering_.get()) {
            throw std::runtime_error("Call failed: renderer::CommandBufferRenderPassPeriod::bindIndexBuffer(): Render pass has ended");
        }

        VkIndexType type;

        switch (indexType) {
            case IndexType::U16:
                type = VK_INDEX_TYPE_UINT16;
                break;

            case IndexType::U32:
                type = VK_INDEX_TYPE_UINT32;
                break;
        }

        vkCmdBindIndexBuffer(commandBuffer_->getVkCommandBuffer(), buffer.getVkBuffer(), offset, type);
    }

    void CommandBufferRenderPassPeriod::setPipelineViewports(const std::vector<renderer::Viewport>& viewports, std::uint32_t offset) {
        if (!rendering_.get()) {
            throw std::runtime_error("Call failed: renderer::CommandBufferRenderPassPeriod::setPipelineViewports(): Render pass has ended");
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

        vkCmdSetViewport(commandBuffer_->getVkCommandBuffer(), offset, static_cast<std::uint32_t>(vulkanViewports.size()), vulkanViewports.data());
    }

    void CommandBufferRenderPassPeriod::setPipelineScissors(const std::vector<renderer::Scissor>& scissors, std::uint32_t offset) {
        if (!rendering_.get()) {
            throw std::runtime_error("Call failed: renderer::CommandBufferRenderPassPeriod::setPipelineScissors(): Render pass has ended");
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

        vkCmdSetScissor(commandBuffer_->getVkCommandBuffer(), offset, static_cast<std::uint32_t>(vulkanScissors.size()), vulkanScissors.data());
    }

    void CommandBufferRenderPassPeriod::setPipelineLineWidth(float width) {
        if (!rendering_.get()) {
            throw std::runtime_error("Call failed: renderer::CommandBufferRenderPassPeriod::setPipelineLineWidth(): Render pass has ended");
        }

        vkCmdSetLineWidth(commandBuffer_->getVkCommandBuffer(), width);
    }

    void CommandBufferRenderPassPeriod::setPipelineDepthBias(float depthBiasConstantFactor, float depthBiasClamp, float depthBiasSlopeFactor) {
        if (!rendering_.get()) {
            throw std::runtime_error("Call failed: renderer::CommandBufferRenderPassPeriod::setPipelineDepthBias(): Render pass has ended");
        }

        vkCmdSetDepthBias(commandBuffer_->getVkCommandBuffer(), depthBiasConstantFactor, depthBiasClamp, depthBiasSlopeFactor);
    }

    void CommandBufferRenderPassPeriod::setPipelineBlendConstants(BlendConstants blend) {
        if (!rendering_.get()) {
            throw std::runtime_error("Call failed: renderer::CommandBufferRenderPassPeriod::setPipelineBlendConstants(): Render pass has ended");
        }

        vkCmdSetBlendConstants(commandBuffer_->getVkCommandBuffer(), &blend.r);
    }

    void CommandBufferRenderPassPeriod::setPipelineDepthBounds(data::Range<float> range) {
        if (!rendering_.get()) {
            throw std::runtime_error("Call failed: renderer::CommandBufferRenderPassPeriod::setPipelineDepthBounds(): Render pass has ended");
        }

        vkCmdSetDepthBounds(commandBuffer_->getVkCommandBuffer(), range.min, range.max);
    }

    void CommandBufferRenderPassPeriod::setPipelineStencilCompareMask(StencilFaces faces, std::uint32_t compareMask) {
        if (!rendering_.get()) {
            throw std::runtime_error("Call failed: renderer::CommandBufferRenderPassPeriod::setPipelineStencilCompareMask(): Render pass has ended");
        }

        VkStencilFaceFlags face = 0;

        switch (faces) {
            case StencilFaces::FRONT:
                face = VK_STENCIL_FACE_FRONT_BIT;
                break;

            case StencilFaces::BACK:
                face = VK_STENCIL_FACE_BACK_BIT;
                break;

            case StencilFaces::BOTH:
                face = VK_STENCIL_FACE_FRONT_AND_BACK;
                break;
        }

        vkCmdSetStencilCompareMask(commandBuffer_->getVkCommandBuffer(), face, compareMask);
    }

    void CommandBufferRenderPassPeriod::setPipelineStencilWriteMask(StencilFaces faces, std::uint32_t writeMask) {
        if (!rendering_.get()) {
            throw std::runtime_error("Call failed: renderer::CommandBufferRenderPassPeriod::setPipelineStencilWriteMask(): Render pass has ended");
        }

        VkStencilFaceFlags face = 0;

        switch (faces) {
            case StencilFaces::FRONT:
                face = VK_STENCIL_FACE_FRONT_BIT;
                break;

            case StencilFaces::BACK:
                face = VK_STENCIL_FACE_BACK_BIT;
                break;

            case StencilFaces::BOTH:
                face = VK_STENCIL_FACE_FRONT_AND_BACK;
                break;
        }

        vkCmdSetStencilWriteMask(commandBuffer_->getVkCommandBuffer(), face, writeMask);
    }

    void CommandBufferRenderPassPeriod::setPipelineStencilReferenceMask(StencilFaces faces, std::uint32_t reference) {
        if (!rendering_.get()) {
            throw std::runtime_error("Call failed: renderer::CommandBufferRenderPassPeriod::setPipelineStencilReferenceMask(): Render pass has ended");
        }

        VkStencilFaceFlags face = 0;

        switch (faces) {
            case StencilFaces::FRONT:
                face = VK_STENCIL_FACE_FRONT_BIT;
                break;

            case StencilFaces::BACK:
                face = VK_STENCIL_FACE_BACK_BIT;
                break;

            case StencilFaces::BOTH:
                face = VK_STENCIL_FACE_FRONT_AND_BACK;
                break;
        }

        vkCmdSetStencilReference(commandBuffer_->getVkCommandBuffer(), face, reference);
    }

    void CommandBufferRenderPassPeriod::pushConstants(PipelineLayout& layout, std::uint32_t stageFlags, std::span<std::uint8_t> data, std::uint32_t offset) {
        if (!rendering_.get()) {
            throw std::runtime_error("Call failed: renderer::CommandBufferRenderPassPeriod::pushConstants(): Render pass has ended");
        }

        VkShaderStageFlags flags = 0;

        if (stageFlags & DescriptorShaderStageFlags::VERTEX) {
            flags |= VK_SHADER_STAGE_VERTEX_BIT;
        }

        if (stageFlags & DescriptorShaderStageFlags::FRAGMENT) {
            flags |= VK_SHADER_STAGE_FRAGMENT_BIT;
        }

        vkCmdPushConstants(commandBuffer_->getVkCommandBuffer(), layout.getVkPipelineLayout(), flags, offset, static_cast<std::uint32_t>(data.size()), data.data());
    }

    void CommandBufferRenderPassPeriod::draw(std::uint32_t vertexCount, std::uint32_t instances, std::uint32_t firstVertex, std::uint32_t firstInstance) {
        if (!rendering_.get()) {
            throw std::runtime_error("Call failed: renderer::CommandBufferRenderPassPeriod::draw(): Render pass has ended");
        }

        vkCmdDraw(commandBuffer_->getVkCommandBuffer(), vertexCount, instances, firstVertex, firstInstance);
    }

    void CommandBufferRenderPassPeriod::drawIndexed(std::uint32_t indexCount, std::uint32_t instanceCount, std::uint32_t firstIndex, std::uint32_t firstInstance, std::int32_t vertexOffset) {
        if (!rendering_.get()) {
            throw std::runtime_error("Call failed: renderer::CommandBufferRenderPassPeriod::drawIndexed(): Render pass has ended");
        }

        vkCmdDrawIndexed(commandBuffer_->getVkCommandBuffer(), indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
    }

    bool CommandBufferRenderPassPeriod::renderEnded() const {
        return !rendering_.get();
    }

    CommandBufferRenderPassPeriod::CommandBufferRenderPassPeriod(CommandBufferCapturePeriod& period, RenderPassBeginInfo& beginInfo)
        : commandBuffer_(period.commandBuffer_), rendering_(period.rendering_) {
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

        vkCmdBeginRenderPass(commandBuffer_->getVkCommandBuffer(), &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
    }

    CommandBufferRenderPassPeriod CommandBufferCapturePeriod::beginRenderPass(RenderPassBeginInfo& beginInfo) {
        rendering_ = true;

        return CommandBufferRenderPassPeriod(*this, beginInfo);
    }

    CommandBufferCapturePeriod::~CommandBufferCapturePeriod() {
        end();
    }

    void CommandBufferCapturePeriod::end() {
        if (capturing_.get() && !rendering_) {
            capturing_.get() = false;

            if (vkEndCommandBuffer(commandBuffer_->commandBuffer_) != VK_SUCCESS) {
                throw std::runtime_error("Call failed: renderer::CommandBufferCapturePeriod::end(): Failed to end command buffer capture");
            }
        }
    }

    void CommandBufferCapturePeriod::copyBuffer(Buffer& source, Buffer& destination, const std::vector<BufferCopyRegion>& copyRegions) {
        if (!capturing_.get()) {
            throw std::runtime_error("Call failed: renderer::CommandBufferCapturePeriod::copyBuffer(): Render pass has ended");
        }

        std::vector<VkBufferCopy> bufferCopies(copyRegions.size());

        for (std::size_t i = 0; i < bufferCopies.size(); i++) {
            auto& bufferCopy = bufferCopies[i];
            auto& copyRegion = copyRegions[i];

            bufferCopy = {
                .srcOffset = copyRegion.sourceOffset,
                .dstOffset = copyRegion.destinationOffset,
                .size = copyRegion.size,
            };
        }

        vkCmdCopyBuffer(commandBuffer_->getVkCommandBuffer(), source.getVkBuffer(), destination.getVkBuffer(), static_cast<std::uint32_t>(bufferCopies.size()), bufferCopies.data());
    }

    void CommandBufferCapturePeriod::bindDescriptorSets(DescriptorSetBindPoint bindPoint, PipelineLayout& layout, std::uint32_t firstSet, const std::vector<data::Reference<DescriptorSet>>& sets) {
        if (!capturing_.get()) {
            throw std::runtime_error("Call failed: renderer::CommandBufferCapturePeriod::bindDescriptorSets(): Render pass has ended");
        }

        VkPipelineBindPoint point;

        switch (bindPoint) {
            case renderer::DescriptorSetBindPoint::RENDER:
                point = VK_PIPELINE_BIND_POINT_GRAPHICS;
                break;

            case renderer::DescriptorSetBindPoint::COMPUTE:
                point = VK_PIPELINE_BIND_POINT_COMPUTE;
                break;
        }

        std::vector<VkDescriptorSet> vkSets(sets.size());

        for (std::size_t i = 0; i < vkSets.size(); i++) {
            vkSets[i] = sets[i]->getVkDescriptorSet();
        }

        vkCmdBindDescriptorSets(commandBuffer_->getVkCommandBuffer(), point, layout.getVkPipelineLayout(), firstSet, static_cast<std::uint32_t>(vkSets.size()), vkSets.data(), 0, nullptr);
    }

    bool CommandBufferCapturePeriod::isRendering() const {
        return rendering_;
    }

    bool CommandBufferCapturePeriod::captureEnded() const {
        return !capturing_.get();
    }

    CommandBufferCapturePeriod::CommandBufferCapturePeriod(CommandBuffer& commandBuffer)
        : commandBuffer_(commandBuffer), capturing_(commandBuffer.capturing_) {
        VkCommandBufferBeginInfo commandBufferBeginInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .pNext = nullptr,
            .flags = 0,
            .pInheritanceInfo = nullptr,
        };

        if (vkBeginCommandBuffer(commandBuffer_->commandBuffer_, &commandBufferBeginInfo) != VK_SUCCESS) {
            throw std::runtime_error("Construction failed: renderer::CommandBufferCapturePeriod: Failed to start command buffer capture");
        }
    }

    CommandBuffer::~CommandBuffer() {
    }

    void CommandBuffer::reset() {
        vkResetCommandBuffer(commandBuffer_, 0);
    }

    CommandBufferCapturePeriod CommandBuffer::beginCapture() {
        capturing_ = true;

        return CommandBufferCapturePeriod(*this);
    }

    bool CommandBuffer::isCapturing() const {
        return capturing_;
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