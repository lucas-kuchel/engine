#pragma once

#include <data/references.hpp>

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
        ShaderModule(const ShaderModuleCreateInfo& createInfo);
        ~ShaderModule();

        ShaderModule(const ShaderModule&) = delete;
        ShaderModule(ShaderModule&&) noexcept = default;

        ShaderModule& operator=(const ShaderModule&) = delete;
        ShaderModule& operator=(ShaderModule&&) noexcept = default;

        [[nodiscard]] VkShaderModule& getVkShaderModule();

        [[nodiscard]] const VkShaderModule& getVkShaderModule() const;

    private:
        VkShaderModule module_ = VK_NULL_HANDLE;

        data::Ref<Device> device_;
    };
}