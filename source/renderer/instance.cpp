#include <renderer/instance.hpp>

#include <stdexcept>

#include <GLFW/glfw3.h>

namespace renderer {
    Instance::Instance() {
        std::uint32_t apiVersion = 0;

        if (vkEnumerateInstanceVersion(&apiVersion) != VK_SUCCESS) {
            throw std::runtime_error("Error calling renderer::Instance::create(): Could not retrieve API version");
        }

        VkApplicationInfo applicationInfo = {
            .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
            .pNext = nullptr,
            .pApplicationName = "generic",
            .applicationVersion = VK_MAKE_VERSION(0, 0, 1),
            .pEngineName = "generic",
            .engineVersion = VK_MAKE_VERSION(0, 0, 1),
            .apiVersion = apiVersion,
        };

        std::uint32_t extensionCount = 0;

        const char** extensions = glfwGetRequiredInstanceExtensions(&extensionCount);

        std::uint32_t availableLayerCount = 0;

        if (vkEnumerateInstanceLayerProperties(&availableLayerCount, nullptr) != VK_SUCCESS) {
            throw std::runtime_error("failed to enumerate Vulkan instance layer properties");
        }

        std::vector<VkLayerProperties> layerProperties(availableLayerCount);
        std::vector<const char*> selectedLayers;

        if (vkEnumerateInstanceLayerProperties(&availableLayerCount, layerProperties.data()) != VK_SUCCESS) {
            throw std::runtime_error("failed to enumerate Vulkan instance layer properties");
        }

#if defined(DEBUG_BUILD_TYPE)
        for (auto& instanceLayer : layerProperties) {
            bool isValidationLayer = std::string_view(instanceLayer.layerName) == "VK_LAYER_KHRONOS_validation";

            if (isValidationLayer) {
                selectedLayers.push_back(instanceLayer.layerName);
            }
        }
#endif

        std::uint32_t layerCount = static_cast<std::uint32_t>(selectedLayers.size());

        VkInstanceCreateInfo instanceCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .pApplicationInfo = &applicationInfo,
            .enabledLayerCount = layerCount,
            .ppEnabledLayerNames = selectedLayers.data(),
            .enabledExtensionCount = extensionCount,
            .ppEnabledExtensionNames = extensions,
        };

        if (vkCreateInstance(&instanceCreateInfo, nullptr, &data_.instance) != VK_SUCCESS) {
            throw std::runtime_error("failed to create VkInstance");
        }

        std::uint32_t physicalDeviceCount = 0;

        if (vkEnumeratePhysicalDevices(data_.instance, &physicalDeviceCount, nullptr) != VK_SUCCESS) {
            throw std::runtime_error("failed to enumerate VkPhysicalDevices");
        }

        std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);

        if (vkEnumeratePhysicalDevices(data_.instance, &physicalDeviceCount, physicalDevices.data()) != VK_SUCCESS) {
            throw std::runtime_error("failed to enumerate VkPhysicalDevices");
        }

        for (auto& device : physicalDevices) {
            VkPhysicalDeviceProperties deviceProperties;

            vkGetPhysicalDeviceProperties(device, &deviceProperties);

            if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
                data_.physicalDevice = device;
                break;
            }
        }

        if (data_.physicalDevice == VK_NULL_HANDLE) {
            data_.physicalDevice = physicalDevices.front();
        }

        std::uint32_t queueFamilyCount = 0;

        vkGetPhysicalDeviceQueueFamilyProperties(data_.physicalDevice, &queueFamilyCount, nullptr);

        data_.queueFamilyProperties.resize(queueFamilyCount);
        data_.queueFamilyOccupations.resize(queueFamilyCount);

        vkGetPhysicalDeviceQueueFamilyProperties(data_.physicalDevice, &queueFamilyCount, data_.queueFamilyProperties.data());
    }

    Instance::~Instance() {
        if (data_.instance != VK_NULL_HANDLE) {
            vkDestroyInstance(data_.instance, nullptr);
        }
    }

    InstanceData& Instance::getData() {
        return data_;
    }
}