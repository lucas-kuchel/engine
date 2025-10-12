#include <renderer/device.hpp>
#include <renderer/shader_module.hpp>

namespace renderer {
    ShaderModule ShaderModule::create(const ShaderModuleCreateInfo& createInfo) {
        ShaderModule shaderModule;

        VkShaderModuleCreateInfo shaderModuleCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .codeSize = static_cast<std::uint32_t>(createInfo.data.size() * sizeof(std::uint32_t)),
            .pCode = createInfo.data.data(),
        };

        if (vkCreateShaderModule(createInfo.device.device_, &shaderModuleCreateInfo, nullptr, &shaderModule.module_) != VK_SUCCESS) {
            shaderModule.module_ = nullptr;
        }
        else {
            shaderModule.device_ = &createInfo.device;
        }

        return shaderModule;
    }

    void ShaderModule::destroy(ShaderModule& shaderModule) {
        if (shaderModule.module_) {
            vkDestroyShaderModule(shaderModule.device_->device_, shaderModule.module_, nullptr);
        }
    }
}