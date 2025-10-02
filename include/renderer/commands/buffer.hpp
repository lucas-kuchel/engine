#pragma once

#include <data/references.hpp>

#include <renderer/resources/buffer.hpp>
#include <renderer/resources/pipeline.hpp>

#include <vulkan/vulkan.h>

namespace renderer {
    class Device;
    class CommandPool;
    class CommandBuffer;
    class CommandBufferCapturePeriod;

    struct RenderPassBeginInfo;

    // @brief Represents a render pass period for a command buffer
    // @note Not safe to copy
    class CommandBufferRenderPassPeriod {
    public:
        ~CommandBufferRenderPassPeriod();

        CommandBufferRenderPassPeriod(const CommandBufferRenderPassPeriod&) = delete;
        CommandBufferRenderPassPeriod(CommandBufferRenderPassPeriod&&) noexcept = default;

        CommandBufferRenderPassPeriod& operator=(const CommandBufferRenderPassPeriod&) = delete;
        CommandBufferRenderPassPeriod& operator=(CommandBufferRenderPassPeriod&&) noexcept = default;

        // @brief Ends the render pass prematurely
        // @note Not necessary to call; destructor will call it too
        void end();

        void bindPipeline(Pipeline& pipeline);
        void bindVertexBuffers(const std::vector<data::Reference<Buffer>>& buffers, const std::vector<std::uint64_t>& offsets, std::uint32_t first);
        void bindIndexBuffer(Buffer& buffer, std::uint64_t offset, IndexType indexType);

        void setPipelineViewports(const std::vector<renderer::Viewport>& viewports, std::uint32_t offset);
        void setPipelineScissors(const std::vector<renderer::Scissor>& scissors, std::uint32_t offset);
        void setPipelineLineWidth(float width);
        void setPipelineDepthBias(float depthBiasConstantFactor, float depthBiasClamp, float depthBiasSlopeFactor);
        void setPipelineBlendConstants(BlendConstants blend);
        void setPipelineDepthBounds(data::Range<float> range);
        void setPipelineStencilCompareMask(StencilFaces faces, std::uint32_t compareMask);
        void setPipelineStencilWriteMask(StencilFaces faces, std::uint32_t writeMask);
        void setPipelineStencilReferenceMask(StencilFaces faces, std::uint32_t reference);

        void pushConstants(PipelineLayout& layout, std::uint32_t stageFlags, std::span<std::uint8_t> data, std::uint32_t offset);

        void draw(std::uint32_t vertexCount, std::uint32_t instances, std::uint32_t firstVertex, std::uint32_t firstInstance);
        void drawIndexed(std::uint32_t indexCount, std::uint32_t instanceCount, std::uint32_t firstIndex, std::uint32_t firstInstance, std::int32_t vertexOffset);

        // @brief Indicates if this render pass has ended
        [[nodiscard]] bool renderEnded() const;

    private:
        CommandBufferRenderPassPeriod(CommandBufferCapturePeriod& period, RenderPassBeginInfo& beginInfo);

        data::Reference<CommandBuffer> commandBuffer_;
        data::Reference<bool> rendering_;

        friend class CommandBufferCapturePeriod;
    };

    // @brief Represents a command capture period for a command buffer
    // @note Not safe to copy
    class CommandBufferCapturePeriod {
    public:
        ~CommandBufferCapturePeriod();

        CommandBufferCapturePeriod(const CommandBufferCapturePeriod&) = delete;
        CommandBufferCapturePeriod(CommandBufferCapturePeriod&&) noexcept = default;

        CommandBufferCapturePeriod& operator=(const CommandBufferCapturePeriod&) = delete;
        CommandBufferCapturePeriod& operator=(CommandBufferCapturePeriod&&) noexcept = default;

        // @brief Starts a new render pass
        // @note Only one render pass can occur at a time for a given command recording
        // @param The begin info for the render pass
        // @return The render pass period object
        // @throws std::runtime_error if called when already rendering
        [[nodiscard]] CommandBufferRenderPassPeriod beginRenderPass(RenderPassBeginInfo& beginInfo);

        // @brief Ends the capture prematurely
        // @note Not necessary to call; destructor will call it too
        void end();

        void copyBuffer(Buffer& source, Buffer& destination, const std::vector<BufferCopyRegion>& copyRegions);

        // @brief Indicates if command capture is already rendering
        // @return If the command capture is rendering
        [[nodiscard]] bool isRendering() const;

        // @brief Indicates if this capture has ended
        [[nodiscard]] bool captureEnded() const;

    private:
        CommandBufferCapturePeriod(CommandBuffer& commandBuffer);

        bool rendering_ = false;

        data::Reference<CommandBuffer> commandBuffer_;
        data::NullableReference<bool> capturing_;

        friend class CommandBufferRenderPassPeriod;
        friend class CommandBuffer;
    };

    // @brief Represents a buffer for submitting commands to
    // @note Not safe to copy
    class CommandBuffer {
    public:
        ~CommandBuffer();

        CommandBuffer(const CommandBuffer&) = delete;
        CommandBuffer(CommandBuffer&&) noexcept = default;

        CommandBuffer& operator=(const CommandBuffer&) = delete;
        CommandBuffer& operator=(CommandBuffer&&) noexcept = default;

        // @brief Resets the command buffer's contents
        void reset();

        // @brief Starts a new capture period for commands
        // @note Only one capture period can occur at a time for a given command buffer
        // @return The capture period object
        // @throws std::runtime_error if called when already capturing
        [[nodiscard]] CommandBufferCapturePeriod beginCapture();

        // @brief Indicates if command buffer is already capturing
        // @return If the command buffer is capturing
        [[nodiscard]] bool isCapturing() const;

        // @brief Provides the Vulkan VkCommandBuffer
        // @return The VkCommandBuffer
        [[nodiscard]] VkCommandBuffer& getVkCommandBuffer();

        // @brief Provides the Vulkan VkCommandBuffer
        // @return The VkCommandBuffer
        [[nodiscard]] const VkCommandBuffer& getVkCommandBuffer() const;

    private:
        CommandBuffer(CommandPool& comandPool);

        VkCommandBuffer commandBuffer_;

        data::Reference<CommandPool> commandPool_;

        bool capturing_ = false;

        friend class CommandBufferCapturePeriod;
        friend class CommandPool;
        friend class std::vector<CommandPool>;
    };
}