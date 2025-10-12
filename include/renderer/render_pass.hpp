#pragma once

#include <renderer/configuration.hpp>

#include <optional>
#include <vector>

#include <glm/glm.hpp>

#include <vulkan/vulkan.h>

namespace renderer {
    class Framebuffer;
    class Device;

    struct FrameAttachmentOperationInfo {
        LoadOperation load;
        StoreOperation store;
    };

    // @brief Information for a frame attachment
    struct ColourAttachmentInfo {
        ImageFormat format;
        ImageLayout initialLayout;
        ImageLayout finalLayout;
        FrameAttachmentOperationInfo operations;
    };

    struct DepthStencilInfo {
        ImageFormat format;
        ImageLayout initialLayout;
        ImageLayout finalLayout;
        FrameAttachmentOperationInfo depthOperations;
        FrameAttachmentOperationInfo stencilOperations;
    };

    struct SubpassInfo {
        std::vector<std::uint32_t> colourAttachmentInputIndices;
        std::vector<std::uint32_t> colourAttachmentOutputIndices;

        std::optional<std::uint32_t> depthStencilIndex;
    };

    struct SubpassDependencyInfo {
        Flags stageSourceFlags;
        Flags stageDestinationFlags;

        Flags accessSourceFlags;
        Flags accessDestinationFlags;

        std::optional<std::uint32_t> subpassSourceIndex;
        std::optional<std::uint32_t> subpassDestinationIndex;
    };

    // @brief Creation information for a render pass
    struct RenderPassCreateInfo {
        Device& device;

        std::vector<DepthStencilInfo> depthStencilAttachments;
        std::vector<ColourAttachmentInfo> colourAttachments;
        std::vector<SubpassInfo> subpasses;
        std::vector<SubpassDependencyInfo> subpassDependencies;

        std::uint32_t sampleCount;
    };

    class RenderPass {
    public:
        static RenderPass create(const RenderPassCreateInfo& createInfo);
        static void destroy(RenderPass& renderPass);

    private:
        VkRenderPass renderPass_ = nullptr;
        Device* device_ = nullptr;

        friend class CommandBuffer;
        friend class Device;
        friend class Framebuffer;
    };

    struct RenderPassRegion {
        // @brief The position of the top-left corner
        glm::ivec2 position = {};

        // @brief The extent (size) of the renderable region
        glm::uvec2 extent = {};
    };

    // @brief Information regarding starting a render pass
    struct RenderPassBeginInfo {
        RenderPass& renderPass;
        Framebuffer& framebuffer;

        RenderPassRegion region;

        std::vector<glm::fvec4> colourClearValues;

        std::optional<float> depthClearValue;
        std::optional<std::uint32_t> stencilClearValue;
    };
}