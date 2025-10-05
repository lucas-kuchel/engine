#include <renderer/resources/sampler.hpp>

#include <renderer/device.hpp>

namespace renderer {
    Sampler::Sampler(const SamplerCreateInfo& createInfo)
        : device_(createInfo.device) {
        VkFilter min;
        VkFilter mag;

        switch (createInfo.minFilter) {
            case Filter::LINEAR:
                min = VK_FILTER_LINEAR;
                break;

            case Filter::NEAREST:
                min = VK_FILTER_NEAREST;
                break;
        }

        switch (createInfo.magFilter) {
            case Filter::LINEAR:
                mag = VK_FILTER_LINEAR;
                break;

            case Filter::NEAREST:
                mag = VK_FILTER_NEAREST;
                break;
        }

        VkSamplerMipmapMode mipmapMode;

        switch (createInfo.mipmapMode) {
            case MipmapMode::LINEAR:
                mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
                break;

            case MipmapMode::NEAREST:
                mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
                break;
        }

        VkSamplerAddressMode u;
        VkSamplerAddressMode v;
        VkSamplerAddressMode w;

        switch (createInfo.addressModeU) {
            case AddressMode::CLAMP_TO_BORDER:
                u = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
                break;

            case AddressMode::CLAMP_TO_EDGE:
                u = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
                break;

            case AddressMode::REPEAT:
                u = VK_SAMPLER_ADDRESS_MODE_REPEAT;
                break;

            case AddressMode::MIRRORED_REPEAT:
                u = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
                break;

            case AddressMode::MIRROR_CLAMP_TO_EDGE:
                u = VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE;
                break;
        }

        switch (createInfo.addressModeV) {
            case AddressMode::CLAMP_TO_BORDER:
                v = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
                break;

            case AddressMode::CLAMP_TO_EDGE:
                v = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
                break;

            case AddressMode::REPEAT:
                v = VK_SAMPLER_ADDRESS_MODE_REPEAT;
                break;

            case AddressMode::MIRRORED_REPEAT:
                v = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
                break;

            case AddressMode::MIRROR_CLAMP_TO_EDGE:
                v = VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE;
                break;
        }

        switch (createInfo.addressModeW) {
            case AddressMode::CLAMP_TO_BORDER:
                w = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
                break;

            case AddressMode::CLAMP_TO_EDGE:
                w = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
                break;

            case AddressMode::REPEAT:
                w = VK_SAMPLER_ADDRESS_MODE_REPEAT;
                break;

            case AddressMode::MIRRORED_REPEAT:
                w = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
                break;

            case AddressMode::MIRROR_CLAMP_TO_EDGE:
                w = VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE;
                break;
        }

        VkCompareOp compare;

        switch (createInfo.comparison) {
            case CompareOperation::EQUAL:
                compare = VK_COMPARE_OP_EQUAL;
                break;

            case CompareOperation::NOT_EQUAL:
                compare = VK_COMPARE_OP_NOT_EQUAL;
                break;

            case CompareOperation::GREATER:
                compare = VK_COMPARE_OP_GREATER;
                break;

            case CompareOperation::GREATER_EQUAL:
                compare = VK_COMPARE_OP_GREATER_OR_EQUAL;
                break;

            case CompareOperation::LESS:
                compare = VK_COMPARE_OP_LESS;
                break;

            case CompareOperation::LESS_EQUAL:
                compare = VK_COMPARE_OP_LESS_OR_EQUAL;
                break;

            case CompareOperation::ALWAYS:
                compare = VK_COMPARE_OP_ALWAYS;
                break;

            case CompareOperation::NEVER:
                compare = VK_COMPARE_OP_NEVER;
                break;
        }

        VkBorderColor border;

        switch (createInfo.borderColour) {
            case BorderColour::FLOAT_TRANSPARENT_BLACK:
                border = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
                break;

            case BorderColour::FLOAT_OPAQUE_BLACK:
                border = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
                break;

            case BorderColour::FLOAT_OPAQUE_WHITE:
                border = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
                break;

            case BorderColour::INT_TRANSPARENT_BLACK:
                border = VK_BORDER_COLOR_INT_TRANSPARENT_BLACK;
                break;

            case BorderColour::INT_OPAQUE_BLACK:
                border = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
                break;

            case BorderColour::INT_OPAQUE_WHITE:
                border = VK_BORDER_COLOR_INT_OPAQUE_WHITE;
                break;
        }

        VkSamplerCreateInfo samplerCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .magFilter = mag,
            .minFilter = min,
            .mipmapMode = mipmapMode,
            .addressModeU = u,
            .addressModeV = v,
            .addressModeW = w,
            .mipLodBias = createInfo.mipLodBias,
            .anisotropyEnable = createInfo.enableAnisotropy,
            .maxAnisotropy = createInfo.maxAnisotropy,
            .compareEnable = createInfo.enableCompare,
            .compareOp = compare,
            .minLod = createInfo.minLod,
            .maxLod = createInfo.maxLod,
            .borderColor = border,
            .unnormalizedCoordinates = createInfo.unnormalisedCoordinates,
        };

        if (vkCreateSampler(device_->getVkDevice(), &samplerCreateInfo, nullptr, &sampler_) != VK_SUCCESS) {
            throw std::runtime_error("Construction failed: renderer::Sampler: Failed to create sampler");
        }
    }

    Sampler::~Sampler() {
        if (sampler_) {
            vkDestroySampler(device_->getVkDevice(), sampler_, nullptr);
        }
    }

    VkSampler& Sampler::getVkSampler() {
        return sampler_;
    }

    const VkSampler& Sampler::getVkSampler() const {
        return sampler_;
    }
}