#pragma once

#include <cstdint>
#include <vector>

#include <vulkan/vulkan.h>

namespace renderer {

    struct InstanceData {
        VkInstance instance = VK_NULL_HANDLE;
        VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

        std::vector<VkQueueFamilyProperties> queueFamilyProperties;
        std::vector<std::uint32_t> queueFamilyOccupations;
    };

    class Instance {
    public:
        Instance();
        ~Instance();

        Instance(const Instance&) = delete;
        Instance(Instance&&) noexcept = default;

        Instance& operator=(const Instance&) = delete;
        Instance& operator=(Instance&&) noexcept = default;

        InstanceData& getData();

    private:
        InstanceData data_;
    };
}