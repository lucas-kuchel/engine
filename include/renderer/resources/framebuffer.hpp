#pragma once

#include <data/extent.hpp>
#include <data/references.hpp>

#include <vulkan/vulkan.h>

namespace renderer {
    class Device;
    class ImageView;
    class RenderPass;

    // @brief Creation information for a framebuffer
    struct FramebufferCreateInfo {
        Device& device;
        RenderPass& renderPass;

        data::ReferenceList<ImageView> imageViews;
    };

    // @brief Represents the images provided to a render pass
    // @note Not safe to copy
    class Framebuffer {
    public:
        Framebuffer(const FramebufferCreateInfo& createInfo);
        ~Framebuffer();

        Framebuffer(const Framebuffer&) = delete;
        Framebuffer(Framebuffer&&) noexcept = default;

        Framebuffer& operator=(const Framebuffer&) = delete;
        Framebuffer& operator=(Framebuffer&&) noexcept = default;

        // @brief Provides the Vulkan VkFramebuffer
        // @return The VkFramebuffer
        [[nodiscard]] VkFramebuffer& getVkFramebuffer();

        // @brief Provides the Vulkan VkFramebuffer
        // @return The VkFramebuffer
        [[nodiscard]] const VkFramebuffer& getVkFramebuffer() const;

    private:
        VkFramebuffer framebuffer_ = VK_NULL_HANDLE;

        data::Ref<Device> device_;
    };
}