#include <renderer/instance.hpp>

#include <print>
#include <stdexcept>
#include <string_view>

#include <GLFW/glfw3.h>

namespace renderer {
    Instance::Instance(const InstanceCreateInfo& createInfo) {
        if (vkEnumerateInstanceVersion(&apiVersion_) != VK_SUCCESS) {
            throw std::runtime_error("Error constucting renderer::Instance: Could not retrieve API version");
        }

        auto applicationVersion = VK_MAKE_VERSION(
            createInfo.applicationVersion.major,
            createInfo.applicationVersion.minor,
            createInfo.applicationVersion.patch);

        auto engineVersion = VK_MAKE_VERSION(
            createInfo.engineVersion.major,
            createInfo.engineVersion.minor,
            createInfo.engineVersion.patch);

        VkApplicationInfo applicationInfo = {
            .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
            .pNext = nullptr,
            .pApplicationName = createInfo.applicationName.c_str(),
            .applicationVersion = applicationVersion,
            .pEngineName = createInfo.engineName.c_str(),
            .engineVersion = engineVersion,
            .apiVersion = apiVersion_,
        };

        std::uint32_t windowExtensionCount = 0;

        const char** windowExtensions = glfwGetRequiredInstanceExtensions(&windowExtensionCount);

        std::uint32_t availableExtensionCount = 0;

        if (vkEnumerateInstanceExtensionProperties(nullptr, &availableExtensionCount, nullptr) != VK_SUCCESS) {
            throw std::runtime_error("Error constucting renderer::Instance: Failed to enumerate Vulkan instance extension properties");
        }

        std::vector<VkExtensionProperties> extensionProperties(availableExtensionCount);
        std::vector<std::string> requestedExtensions;
        std::vector<const char*> selectedExtensions;

        if (vkEnumerateInstanceExtensionProperties(nullptr, &availableExtensionCount, extensionProperties.data()) != VK_SUCCESS) {
            throw std::runtime_error("Error constucting renderer::Instance: Failed to enumerate Vulkan instance extension properties");
        }

        for (std::size_t i = 0; i < windowExtensionCount; i++) {
            requestedExtensions.emplace_back(windowExtensions[i]);
        }

        for (std::size_t i = 0; i < extensionProperties.size(); i++) {
            auto& available = extensionProperties[i];

            for (std::size_t j = 0; j < requestedExtensions.size(); j++) {
                auto& requested = requestedExtensions[j];

                if (std::string_view(available.extensionName) != requested) {
                    continue;
                }

                selectedExtensions.push_back(available.extensionName);

                requestedExtensions[j] = requestedExtensions.back();
                requestedExtensions.pop_back();
            }
        }

        if (createInfo.requestDebug) {
            for (auto& requestedExtension : requestedExtensions) {
                std::println("Runtime warning: Requested extension \"{}\" is unsupported", requestedExtension);
            }
        }

        std::uint32_t availableLayerCount = 0;

        if (vkEnumerateInstanceLayerProperties(&availableLayerCount, nullptr) != VK_SUCCESS) {
            throw std::runtime_error("Error constucting renderer::Instance: Failed to enumerate Vulkan instance layer properties");
        }

        std::vector<VkLayerProperties> layerProperties(availableLayerCount);
        std::vector<const char*> selectedLayers;

        if (vkEnumerateInstanceLayerProperties(&availableLayerCount, layerProperties.data()) != VK_SUCCESS) {
            throw std::runtime_error("Error constucting renderer::Instance: Failed to enumerate Vulkan instance layer properties");
        }

        if (createInfo.requestDebug) {
            for (auto& instanceLayer : layerProperties) {
                bool isValidationLayer = std::string_view(instanceLayer.layerName) == "VK_LAYER_KHRONOS_validation";

                if (isValidationLayer) {
                    selectedLayers.push_back(instanceLayer.layerName);
                }
            }
        }

        std::uint32_t layerCount = static_cast<std::uint32_t>(selectedLayers.size());

        VkInstanceCreateInfo instanceCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .pApplicationInfo = &applicationInfo,
            .enabledLayerCount = layerCount,
            .ppEnabledLayerNames = selectedLayers.data(),
            .enabledExtensionCount = static_cast<std::uint32_t>(selectedExtensions.size()),
            .ppEnabledExtensionNames = selectedExtensions.data(),
        };

        if (vkCreateInstance(&instanceCreateInfo, nullptr, &instance_) != VK_SUCCESS) {
            throw std::runtime_error("Error constucting renderer::Instance: Failed to create VkInstance");
        }

        std::uint32_t physicalDeviceCount = 0;

        if (vkEnumeratePhysicalDevices(instance_, &physicalDeviceCount, nullptr) != VK_SUCCESS) {
            throw std::runtime_error("Error constucting renderer::Instance: Failed to enumerate VkPhysicalDevices");
        }

        std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);

        if (vkEnumeratePhysicalDevices(instance_, &physicalDeviceCount, physicalDevices.data()) != VK_SUCCESS) {
            throw std::runtime_error("Error constucting renderer::Instance: Failed to enumerate VkPhysicalDevices");
        }

        for (auto& device : physicalDevices) {
            VkPhysicalDeviceProperties deviceProperties;

            vkGetPhysicalDeviceProperties(device, &deviceProperties);

            if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
                physicalDevice_ = device;
                break;
            }
        }

        if (physicalDevice_ == VK_NULL_HANDLE) {
            physicalDevice_ = physicalDevices.front();
        }

        std::uint32_t queueFamilyCount = 0;

        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice_, &queueFamilyCount, nullptr);

        queueFamilyProperties_.resize(queueFamilyCount);
        queueFamilyOccupations_.resize(queueFamilyCount);

        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice_, &queueFamilyCount, queueFamilyProperties_.data());

        vkGetPhysicalDeviceMemoryProperties(physicalDevice_, &memoryProperties_);
        vkGetPhysicalDeviceProperties(physicalDevice_, &properties_);
    }

    Instance::~Instance() {
        if (instance_ != VK_NULL_HANDLE) {
            vkDestroyInstance(instance_, nullptr);
        }
    }

    VkInstance& Instance::getVkInstance() {
        return instance_;
    }

    VkPhysicalDevice& Instance::getVkPhysicalDevice() {
        return physicalDevice_;
    }

    const VkInstance& Instance::getVkInstance() const {
        return instance_;
    }

    const VkPhysicalDevice& Instance::getVkPhysicalDevice() const {
        return physicalDevice_;
    }

    VkPhysicalDeviceMemoryProperties& Instance::getVkPhysicalDeviceMemoryProperties() {
        return memoryProperties_;
    }

    const VkPhysicalDeviceMemoryProperties& Instance::getVkPhysicalDeviceMemoryProperties() const {
        return memoryProperties_;
    }

    VkPhysicalDeviceProperties& Instance::getVkPhysicalDeviceProperties() {
        return properties_;
    }

    const VkPhysicalDeviceProperties& Instance::getVkPhysicalDeviceProperties() const {
        return properties_;
    }

    const std::vector<VkQueueFamilyProperties>& Instance::getVkQueueFamilyProperties() const {
        return queueFamilyProperties_;
    }

    const std::vector<std::uint32_t>& Instance::getVkQueueFamilyOccupations() const {
        return queueFamilyOccupations_;
    }
}