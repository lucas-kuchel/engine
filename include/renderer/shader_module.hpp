#pragma once

#include <cstdint>
#include <span>

#include <vulkan/vulkan.h>

namespace renderer {
    class Device;

    struct ShaderModuleCreateInfo {
        Device& device;

        std::span<std::uint32_t> data;
    };

    class ShaderModule {
    public:
        static ShaderModule create(const ShaderModuleCreateInfo& createInfo);
        static void destroy(ShaderModule& shaderModule);

    private:
        VkShaderModule module_ = nullptr;
        Device* device_ = nullptr;

        friend class Device;
    };
}