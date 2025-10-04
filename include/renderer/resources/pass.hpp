#pragma once

#include <renderer/resources/config.hpp>

#include <data/colour.hpp>
#include <data/extent.hpp>
#include <data/optional.hpp>
#include <data/rect.hpp>
#include <data/references.hpp>

#include <vector>

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

        data::Optional<std::uint32_t> depthStencilIndex;
    };

    struct SubpassDependencyInfo {
        Flags stageSourceFlags;
        Flags stageDestinationFlags;

        Flags accessSourceFlags;
        Flags accessDestinationFlags;

        data::Optional<std::uint32_t> subpassSourceIndex;
        data::Optional<std::uint32_t> subpassDestinationIndex;
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

    // @brief Describes the requirements of a rendering session
    // @note Not safe to copy
    class RenderPass {
    public:
        RenderPass(const RenderPassCreateInfo& createInfo);
        ~RenderPass();

        RenderPass(const RenderPass&) = delete;
        RenderPass(RenderPass&&) noexcept = default;

        RenderPass& operator=(const RenderPass&) = delete;
        RenderPass& operator=(RenderPass&&) noexcept = default;

        // @brief Provides the VkRenderPass
        //  @return the VkRenderPass
        [[nodiscard]] VkRenderPass& getVkRenderPass();

        // @brief Provides the VkRenderPass
        //  @return the VkRenderPass
        [[nodiscard]] const VkRenderPass& getVkRenderPass() const;

    private:
        VkRenderPass renderPass_ = VK_NULL_HANDLE;

        data::Reference<Device> device_;
    };

    // @brief Information regarding starting a render pass
    struct RenderPassBeginInfo {
        RenderPass& renderPass;
        Framebuffer& framebuffer;
        data::Rect2D<std::int32_t, std::uint32_t> renderArea;

        std::vector<data::ColourRGBA> clearValues;

        data::Optional<float> depthClearValue;
        data::Optional<std::uint32_t> stencilClearValue;
    };
}