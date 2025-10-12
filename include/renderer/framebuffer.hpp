#pragma once

#include <vector>

#include <vulkan/vulkan.h>

namespace renderer {
    class Device;
    class ImageView;
    class RenderPass;

    // @brief Creation information for a framebuffer
    struct FramebufferCreateInfo {
        Device& device;
        RenderPass& renderPass;

        std::vector<ImageView> imageViews;
    };

    class Framebuffer {
    public:
        static Framebuffer create(const FramebufferCreateInfo& createInfo);
        static void destroy(Framebuffer& framebuffer);

    private:
        VkFramebuffer framebuffer_ = nullptr;
        Device* device_ = nullptr;

        friend class CommandBuffer;
    };
}