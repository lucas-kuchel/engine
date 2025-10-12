#pragma once

#include <renderer/configuration.hpp>

#include <optional>

#include <vulkan/vulkan.h>

namespace renderer {
    class Device;

    struct SamplerCreateInfo {
        Device& device;
        Filter minFilter;
        Filter magFilter;
        MipmapMode mipmapMode;
        AddressMode addressModeU;
        AddressMode addressModeV;
        AddressMode addressModeW;
        BorderColour borderColour;

        std::optional<float> maxAnisotropy;
        std::optional<CompareOperation> comparison;

        bool unnormalisedCoordinates;

        float mipLodBias;
        float minLod;
        float maxLod;
    };

    class Sampler {
    public:
        static Sampler create(const SamplerCreateInfo& createInfo);
        static void destroy(Sampler& sampler);

    private:
        VkSampler sampler_ = nullptr;
        Device* device_ = nullptr;

        friend class DescriptorPool;
    };
}