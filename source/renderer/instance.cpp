#include <renderer/instance.hpp>

#include <stdexcept>

#include <GLFW/glfw3.h>

namespace renderer {
    Instance::Instance(const InstanceCreateInfo& createInfo) {
        std::uint32_t apiVersion = 0;

        if (vkEnumerateInstanceVersion(&apiVersion) != VK_SUCCESS) {
            throw std::runtime_error("Error calling renderer::Instance::create(): Could not retrieve API version");
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

        if (vkCreateInstance(&instanceCreateInfo, nullptr, &instance_) != VK_SUCCESS) {
            throw std::runtime_error("failed to create VkInstance");
        }

        std::uint32_t physicalDeviceCount = 0;

        if (vkEnumeratePhysicalDevices(instance_, &physicalDeviceCount, nullptr) != VK_SUCCESS) {
            throw std::runtime_error("failed to enumerate VkPhysicalDevices");
        }

        std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);

        if (vkEnumeratePhysicalDevices(instance_, &physicalDeviceCount, physicalDevices.data()) != VK_SUCCESS) {
            throw std::runtime_error("failed to enumerate VkPhysicalDevices");
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

    const std::vector<VkQueueFamilyProperties>& Instance::getVkQueueFamilyProperties() const {
        return queueFamilyProperties_;
    }

    const std::vector<std::uint32_t>& Instance::getVkQueueFamilyOccupations() const {
        return queueFamilyOccupations_;
    }
}