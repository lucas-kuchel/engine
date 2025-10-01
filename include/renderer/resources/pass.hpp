#pragma once

#include <data/colour.hpp>
#include <data/extent.hpp>
#include <data/optional.hpp>
#include <data/rect.hpp>
#include <data/references.hpp>

#include <cstdint>
#include <vector>

#include <vulkan/vulkan.h>

namespace renderer {
    class Framebuffer;
    class Device;

    enum class ImageFormat : int;

    // @brief What to do when loading a frame attachment
    enum class LoadOperation {
        LOAD,
        CLEAR,
        DONT_CARE,
    };

    // @brief What to do when storing a frame attachment
    enum class StoreOperation {
        STORE,
        DONT_CARE,
    };

    // @brief Information for a frame attachment
    struct FrameAttachmentInfo {
        ImageFormat format;

        LoadOperation loadOperation;
        StoreOperation storeOperation;

        // @brief How many samples should be taken of the image
        std::uint32_t sampleCount;
    };

    // @brief Creation information for a render pass
    struct RenderPassCreateInfo {
        Device& device;

        // @brief All colour frame attachments to be used by the render pass
        // @note Do not put the depth/stencil attachments here
        std::vector<FrameAttachmentInfo> colourAttachments;

        // @brief Optional frame attachment for the depth buffer
        // @note If format is a depth-stencil pair the stencil attachment must have the same format
        data::Optional<FrameAttachmentInfo> depthAttachment;

        // @brief Optional frame attachment for the stencil buffer
        // @note If format is a depth-stencil pair the depth attachment must have the same format
        data::Optional<FrameAttachmentInfo> stencilAttachment;
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