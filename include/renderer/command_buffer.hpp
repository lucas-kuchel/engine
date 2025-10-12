#pragma once

#include <renderer/configuration.hpp>

#include <cstdint>
#include <span>
#include <vector>

#include <glm/glm.hpp>

#include <vulkan/vulkan.h>

namespace renderer {
    class Device;
    class Buffer;
    class Image;
    class CommandPool;
    class CommandBuffer;
    class CommandBufferCapturePeriod;
    class PipelineLayout;
    class Pipeline;
    class DescriptorSet;

    struct ImageMemoryBarrier;
    struct BufferImageCopyRegion;
    struct BufferCopyRegion;
    struct RenderPassBeginInfo;
    struct Scissor;
    struct Viewport;

    class CommandBuffer {
    public:
        static void reset(CommandBuffer& commandBuffer);

        static bool beginCapture(CommandBuffer& commandBuffer);
        static void beginRenderPass(CommandBuffer& commandBuffer, RenderPassBeginInfo& beginInfo);

        static bool endCapture(CommandBuffer& commandBuffer);
        static void endRenderPass(CommandBuffer& commandBuffer);

        static void copyBuffer(CommandBuffer& commandBuffer, Buffer& source, Buffer& destination, const std::vector<BufferCopyRegion>& copyRegions);
        static void copyBufferToImage(CommandBuffer& commandBuffer, Buffer& source, Image& destination, ImageLayout imageLayout, const std::vector<BufferImageCopyRegion>& copyRegions);

        static void nextSubpass(CommandBuffer& commandBuffer);

        static void pipelineBarrier(CommandBuffer& commandBuffer, Flags sourcePipelineStage, Flags destinationPipelineStage, const std::vector<ImageMemoryBarrier>& memoryBarriers);

        static void bindDescriptorSets(CommandBuffer& commandBuffer, DeviceOperation operation, PipelineLayout& layout, std::uint32_t firstSet, const std::vector<DescriptorSet>& sets);
        static void bindPipeline(CommandBuffer& commandBuffer, Pipeline& pipeline);
        static void bindVertexBuffers(CommandBuffer& commandBuffer, const std::vector<Buffer>& buffers, const std::vector<std::uint64_t>& offsets, std::uint32_t first);
        static void bindIndexBuffer(CommandBuffer& commandBuffer, Buffer& buffer, std::uint64_t offset, IndexType indexType);

        static void setPipelineViewports(CommandBuffer& commandBuffer, const std::vector<renderer::Viewport>& viewports, std::uint32_t offset);
        static void setPipelineScissors(CommandBuffer& commandBuffer, const std::vector<renderer::Scissor>& scissors, std::uint32_t offset);
        static void setPipelineLineWidth(CommandBuffer& commandBuffer, float width);
        static void setPipelineDepthBias(CommandBuffer& commandBuffer, float depthBiasConstantFactor, float depthBiasClamp, float depthBiasSlopeFactor);
        static void setPipelineBlendConstants(CommandBuffer& commandBuffer, const glm::fvec4& blend);
        static void setPipelineDepthBounds(CommandBuffer& commandBuffer, float min, float max);
        static void setPipelineStencilCompareMask(CommandBuffer& commandBuffer, Flags faceFlags, std::uint32_t compareMask);
        static void setPipelineStencilWriteMask(CommandBuffer& commandBuffer, Flags faceFlags, std::uint32_t writeMask);
        static void setPipelineStencilReferenceMask(CommandBuffer& commandBuffer, Flags faceFlags, std::uint32_t reference);

        static void pushConstants(CommandBuffer& commandBuffer, PipelineLayout& layout, std::uint32_t stageFlags, std::span<std::uint8_t> data, std::uint32_t offset);

        static void draw(CommandBuffer& commandBuffer, std::uint32_t vertexCount, std::uint32_t instances, std::uint32_t firstVertex, std::uint32_t firstInstance);
        static void drawIndexed(CommandBuffer& commandBuffer, std::uint32_t indexCount, std::uint32_t instanceCount, std::uint32_t firstIndex, std::uint32_t firstInstance, std::int32_t vertexOffset);

        static bool capturing(CommandBuffer& commandBuffer);
        static bool rendering(CommandBuffer& commandBuffer);

    private:
        VkCommandBuffer commandBuffer_ = nullptr;
        CommandPool* commandPool_ = nullptr;

        bool capturing_ = false;
        bool rendering_ = false;

        friend class CommandPool;
        friend class Queue;
    };
}