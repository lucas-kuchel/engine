#include <renderer/commands/buffer.hpp>
#include <renderer/commands/pool.hpp>

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