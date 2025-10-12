#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include <vulkan/vulkan.h>

namespace renderer {
    // @brief Creation information for a instance
    struct InstanceCreateInfo {
        std::string applicationName;

        std::uint32_t applicationVersionMajor;
        std::uint32_t applicationVersionMinor;
        std::uint32_t applicationVersionPatch;

        std::string engineName;

        std::uint32_t engineVersionMajor;
        std::uint32_t engineVersionMinor;
        std::uint32_t engineVersionPatch;

        bool requestDebug;
    };

    class Instance {
    public:
        static Instance create(const InstanceCreateInfo& createInfo);
        static void destroy(Instance& instance);

    private:
        VkInstance instance_ = nullptr;
        VkPhysicalDevice physicalDevice_ = nullptr;

        VkPhysicalDeviceMemoryProperties memoryProperties_;
        VkPhysicalDeviceProperties properties_;

        std::uint32_t apiVersion_ = 0;

        std::vector<VkQueueFamilyProperties> queueFamilyProperties_;
        std::vector<std::uint32_t> queueFamilyOccupations_;

        friend class Device;
        friend class Buffer;
        friend class Image;
        friend class Surface;
        friend class Swapchain;
    };
}