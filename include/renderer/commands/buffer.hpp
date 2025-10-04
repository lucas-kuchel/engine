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
        void beginCapture();
        void beginRenderPass(RenderPassBeginInfo& beginInfo);

        void endCapture();
        void endRenderPass();

        void copyBuffer(Buffer& source, Buffer& destination, const std::vector<BufferCopyRegion>& copyRegions);

        void nextSubpass();

        void bindDescriptorSets(DeviceOperation operation, PipelineLayout& layout, std::uint32_t firstSet, const data::ReferenceList<DescriptorSet>& sets);
        void bindPipeline(Pipeline& pipeline);
        void bindVertexBuffers(const data::ReferenceList<Buffer>& buffers, const std::vector<std::uint64_t>& offsets, std::uint32_t first);
        void bindIndexBuffer(Buffer& buffer, std::uint64_t offset, IndexType indexType);

        void setPipelineViewports(const std::vector<renderer::Viewport>& viewports, std::uint32_t offset);
        void setPipelineScissors(const std::vector<renderer::Scissor>& scissors, std::uint32_t offset);
        void setPipelineLineWidth(float width);
        void setPipelineDepthBias(float depthBiasConstantFactor, float depthBiasClamp, float depthBiasSlopeFactor);
        void setPipelineBlendConstants(BlendConstants blend);
        void setPipelineDepthBounds(data::Range<float> range);
        void setPipelineStencilCompareMask(Flags faceFlags, std::uint32_t compareMask);
        void setPipelineStencilWriteMask(Flags faceFlags, std::uint32_t writeMask);
        void setPipelineStencilReferenceMask(Flags faceFlags, std::uint32_t reference);

        void pushConstants(PipelineLayout& layout, std::uint32_t stageFlags, std::span<std::uint8_t> data, std::uint32_t offset);

        void draw(std::uint32_t vertexCount, std::uint32_t instances, std::uint32_t firstVertex, std::uint32_t firstInstance);
        void drawIndexed(std::uint32_t indexCount, std::uint32_t instanceCount, std::uint32_t firstIndex, std::uint32_t firstInstance, std::int32_t vertexOffset);

        // @brief Indicates if command buffer is already capturing
        // @return If the command buffer is capturing
        [[nodiscard]] bool capturing() const;

        // @brief Indicates if command buffer is already rendering
        // @return If the command buffer is rendering
        [[nodiscard]] bool rendering() const;

        // @brief Provides the Vulkan VkCommandBuffer
        // @return The VkCommandBuffer
        [[nodiscard]] VkCommandBuffer& getVkCommandBuffer();

        // @brief Provides the Vulkan VkCommandBuffer
        // @return The VkCommandBuffer
        [[nodiscard]] const VkCommandBuffer& getVkCommandBuffer() const;

    private:
        CommandBuffer(CommandPool& comandPool);

        VkCommandBuffer commandBuffer_;

        data::Ref<CommandPool> commandPool_;

        bool capturing_ = false;
        bool rendering_ = false;

        friend class CommandPool;
    };
}