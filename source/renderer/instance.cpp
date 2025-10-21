#include <renderer/instance.hpp>

#include <stdexcept>
#include <string_view>

#include <GLFW/glfw3.h>

namespace renderer {
    Instance Instance::create(const InstanceCreateInfo& createInfo) {
        Instance instance;

        if (vkEnumerateInstanceVersion(&instance.apiVersion_) != VK_SUCCESS) {
            instance.instance_ = nullptr;

            return instance;
        }

        auto applicationVersion = VK_MAKE_VERSION(
            createInfo.applicationVersionMajor,
            createInfo.applicationVersionMinor,
            createInfo.applicationVersionPatch);

        auto engineVersion = VK_MAKE_VERSION(
            createInfo.engineVersionMajor,
            createInfo.engineVersionMinor,
            createInfo.engineVersionPatch);

        VkApplicationInfo applicationInfo = {
            .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
            .pNext = nullptr,
            .pApplicationName = createInfo.applicationName.c_str(),
            .applicationVersion = applicationVersion,
            .pEngineName = createInfo.engineName.c_str(),
            .engineVersion = engineVersion,
            .apiVersion = instance.apiVersion_,
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

        for (std::uint64_t i = 0; i < windowExtensionCount; i++) {
            requestedExtensions.emplace_back(windowExtensions[i]);
        }

#if defined(ENGINE_PLATFORM_APPLE)
        requestedExtensions.emplace_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
#endif

        for (std::uint64_t i = 0; i < extensionProperties.size(); i++) {
            auto& available = extensionProperties[i];

            for (std::uint64_t j = 0; j < requestedExtensions.size(); j++) {
                auto& requested = requestedExtensions[j];

                if (std::string_view(available.extensionName) != requested) {
                    continue;
                }

                selectedExtensions.push_back(available.extensionName);

                requestedExtensions[j] = requestedExtensions.back();
                requestedExtensions.pop_back();
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

#if defined(ENGINE_PLATFORM_APPLE)
        instanceCreateInfo.flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#endif

        if (VkResult result = vkCreateInstance(&instanceCreateInfo, nullptr, &instance.instance_); result != VK_SUCCESS) {
            instance.instance_ = nullptr;

            return instance;
        }

        std::uint32_t physicalDeviceCount = 0;

        if (vkEnumeratePhysicalDevices(instance.instance_, &physicalDeviceCount, nullptr) != VK_SUCCESS) {
            vkDestroyInstance(instance.instance_, nullptr);

            instance.instance_ = nullptr;

            return instance;
        }

        std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);

        if (vkEnumeratePhysicalDevices(instance.instance_, &physicalDeviceCount, physicalDevices.data()) != VK_SUCCESS) {
            vkDestroyInstance(instance.instance_, nullptr);

            instance.instance_ = nullptr;

            return instance;
        }

        for (auto& device : physicalDevices) {
            VkPhysicalDeviceProperties deviceProperties;

            vkGetPhysicalDeviceProperties(device, &deviceProperties);

            if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
                instance.physicalDevice_ = device;
                break;
            }
        }

        if (instance.physicalDevice_ == VK_NULL_HANDLE) {
            instance.physicalDevice_ = physicalDevices.front();
        }

        std::uint32_t queueFamilyCount = 0;

        vkGetPhysicalDeviceQueueFamilyProperties(instance.physicalDevice_, &queueFamilyCount, nullptr);

        instance.queueFamilyProperties_.resize(queueFamilyCount);
        instance.queueFamilyOccupations_.resize(queueFamilyCount);

        vkGetPhysicalDeviceQueueFamilyProperties(instance.physicalDevice_, &queueFamilyCount, instance.queueFamilyProperties_.data());

        vkGetPhysicalDeviceMemoryProperties(instance.physicalDevice_, &instance.memoryProperties_);
        vkGetPhysicalDeviceProperties(instance.physicalDevice_, &instance.properties_);

        return instance;
    }

    void Instance::destroy(Instance& instance) {
        if (instance.instance_) {
            vkDestroyInstance(instance.instance_, nullptr);

            instance.instance_ = nullptr;
        }
    }
}