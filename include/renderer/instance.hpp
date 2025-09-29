#pragma once

#include <data/version.hpp>

#include <cstdint>
#include <string>
#include <vector>

#include <vulkan/vulkan.h>

namespace renderer {
    // @brief Creation information for the instance
    struct InstanceCreateInfo {
        data::Version applicationVersion;
        data::Version engineVersion;

        std::string applicationName;
        std::string engineName;
    };

    // @brief Represents the interaction layer between the system/physical device and application
    // @note Not safe to copy
    class Instance {
    public:
        Instance(const InstanceCreateInfo& createInfo);
        ~Instance();

        Instance(const Instance&) = delete;
        Instance(Instance&&) noexcept = default;

        Instance& operator=(const Instance&) = delete;
        Instance& operator=(Instance&&) noexcept = default;

        // @brief Provides the Vulkan VkInstance
        // @return The VkInstance
        [[nodiscard]] VkInstance& getVkInstance();

        // @brief Provides the Vulkan VkPhysicalDevice
        // @return The VkPhysicalDevice
        [[nodiscard]] VkPhysicalDevice& getVkPhysicalDevice();

        // @brief Provides the Vulkan VkInstance
        // @return The VkInstance
        [[nodiscard]] const VkInstance& getVkInstance() const;

        // @brief Provides the Vulkan VkPhysicalDevice
        // @return The VkPhysicalDevice
        [[nodiscard]] const VkPhysicalDevice& getVkPhysicalDevice() const;

        // @brief Provides the Vulkan VkQueueFamilyProperties list
        // @return The VkQueueFamilyProperties list
        [[nodiscard]] const std::vector<VkQueueFamilyProperties>& getVkQueueFamilyProperties() const;

        // @brief Provides the Vulkan queue family occupations list
        // @return The queue family occupations list
        [[nodiscard]] const std::vector<std::uint32_t>& getVkQueueFamilyOccupations() const;

    private:
        VkInstance instance_ = VK_NULL_HANDLE;
        VkPhysicalDevice physicalDevice_ = VK_NULL_HANDLE;

        std::vector<VkQueueFamilyProperties> queueFamilyProperties_;
        std::vector<std::uint32_t> queueFamilyOccupations_;

        friend class Device;
    };
}