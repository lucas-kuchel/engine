#pragma once

#include <renderer/resources/config.hpp>

#include <data/references.hpp>

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
        bool enableAnisotropy;
        float maxAnisotropy;
        bool enableCompare;
        bool unnormalisedCoordinates;
        CompareOperation comparison;
        float mipLodBias;
        float minLod;
        float maxLod;
    };

    class Sampler {
    public:
        Sampler(const SamplerCreateInfo& createInfo);
        ~Sampler();

        Sampler(const Sampler&) = delete;
        Sampler(Sampler&&) noexcept = default;

        Sampler& operator=(const Sampler&) = delete;
        Sampler& operator=(Sampler&&) noexcept = default;

        [[nodiscard]] VkSampler& getVkSampler();
        [[nodiscard]] const VkSampler& getVkSampler() const;

    private:
        VkSampler sampler_;

        data::Ref<Device> device_;
    };
}