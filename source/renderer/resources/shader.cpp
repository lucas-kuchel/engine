#include <renderer/resources/shader.hpp>

#include <renderer/device.hpp>

namespace renderer {
    ShaderModule::ShaderModule(const ShaderModuleCreateInfo& createInfo)
        : device_(createInfo.device) {
        VkShaderModuleCreateInfo shaderModuleCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .codeSize = static_cast<std::uint32_t>(createInfo.data.size() * sizeof(std::uint32_t)),
            .pCode = createInfo.data.data(),
        };

        if (vkCreateShaderModule(createInfo.device.getVkDevice(), &shaderModuleCreateInfo, nullptr, &module_) != VK_SUCCESS) {
            throw std::runtime_error("Construction failed: renderer::ShaderModule: Failed to create shader module");
        }
    }

    ShaderModule::~ShaderModule() {
        if (module_) {
            vkDestroyShaderModule(device_->getVkDevice(), module_, nullptr);
        }
    }

    VkShaderModule& ShaderModule::getVkShaderModule() {
        return module_;
    }

    const VkShaderModule& ShaderModule::getVkShaderModule() const {
        return module_;
    }
}