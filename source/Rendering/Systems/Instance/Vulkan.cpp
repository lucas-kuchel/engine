#if !defined(PLATFORM_APPLE) && !defined(PLATFORM_WINDOWS) && defined(PLATFORM_UNIX)

#include <Rendering/Systems/Instance.hpp>
#include <Windowing/Systems/Window.hpp>

#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>

using namespace Rendering::Systems;
using namespace Rendering::Resources;

class Instance::Backend
{
public:
    VkInstance Instance;

    std::vector<VkPhysicalDevice> Devices;

    VkPhysicalDevice* SelectedDevice = nullptr;
};

Instance::Instance()
    : mBackend(std::make_unique<Backend>())
{
    if (!glfwVulkanSupported())
    {
        throw std::runtime_error("Rendering::Systems::Instance:\nVulkan is unsupported on this system");
    }

    std::uint32_t extensionCount = 0;

    const char** extensions = glfwGetRequiredInstanceExtensions(&extensionCount);

    uint32_t version;

    if (vkEnumerateInstanceVersion(&version) != VK_SUCCESS)
    {
        version = VK_API_VERSION_1_0;
    }

    VkApplicationInfo applicationDescriptor = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pNext = nullptr,
        .pApplicationName = "app.name",
        .applicationVersion = VK_MAKE_VERSION(0, 0, 1),
        .pEngineName = "engine.vulkan",
        .engineVersion = VK_MAKE_VERSION(0, 0, 1),
        .apiVersion = version,
    };

    VkInstanceCreateInfo instanceDescriptor = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .pApplicationInfo = &applicationDescriptor,
        .enabledLayerCount = 0,
        .ppEnabledLayerNames = nullptr,
        .enabledExtensionCount = extensionCount,
        .ppEnabledExtensionNames = extensions,
    };

    if (vkCreateInstance(&instanceDescriptor, nullptr, &mBackend->Instance) != VK_SUCCESS)
    {
        throw std::runtime_error("Rendering::Systems::Instance:\nVulkan instance creation failed");
    }
}

Instance::~Instance()
{
    vkDestroyInstance(mBackend->Instance, nullptr);
}

std::vector<PhysicalDevice> Instance::QueryPhysicalDevices() const
{
    std::uint32_t physicalDeviceCount = 0;

    if (vkEnumeratePhysicalDevices(mBackend->Instance, &physicalDeviceCount, nullptr) != VK_SUCCESS)
    {
        throw std::runtime_error("Rendering::Systems::Instance:\nFailed to enumerate Vulkan physical devices");
    }

    if (physicalDeviceCount == 0)
    {
        throw std::runtime_error("Rendering::Systems::Instance:\nNo physical devices available");
    }

    mBackend->Devices.resize(physicalDeviceCount);

    if (vkEnumeratePhysicalDevices(mBackend->Instance, &physicalDeviceCount, mBackend->Devices.data()) != VK_SUCCESS)
    {
        throw std::runtime_error("Rendering::Systems::Instance:\nFailed to enumerate Vulkan physical devices");
    }

    std::vector<PhysicalDevice> physicalDevices(physicalDeviceCount);

    for (std::size_t i = 0; i < physicalDeviceCount; i++)
    {
        auto& vulkanDevice = mBackend->Devices[i];
        auto& physicalDevice = physicalDevices[i];

        VkPhysicalDeviceProperties properties;
        VkPhysicalDeviceMemoryProperties memoryProperties;
        VkPhysicalDeviceFeatures2 features2;
        VkPhysicalDeviceRayTracingPipelineFeaturesKHR raytracingFeatures;

        features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
        features2.pNext = &raytracingFeatures;

        raytracingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR;
        raytracingFeatures.pNext = nullptr;

        vkGetPhysicalDeviceProperties(vulkanDevice, &properties);
        vkGetPhysicalDeviceMemoryProperties(vulkanDevice, &memoryProperties);
        vkGetPhysicalDeviceFeatures2(vulkanDevice, &features2);

        switch (properties.deviceType)
        {
            case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
            {
                physicalDevice.Type = PhysicalDeviceType::Discrete;
                break;
            }
            case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
            {
                physicalDevice.Type = PhysicalDeviceType::Integrated;
                break;
            }
            default:
            {
                physicalDevice.Type = PhysicalDeviceType::Software;
                break;
            }
        }

        physicalDevice.Index = i;
        physicalDevice.UnifiedMemory = (memoryProperties.memoryHeapCount == 1) || false;
        physicalDevice.SupportsRaytracing = raytracingFeatures.rayTracingPipeline;
        physicalDevice.Name = properties.deviceName;
        physicalDevice.VendorID = properties.vendorID;
        physicalDevice.DeviceID = properties.deviceID;
    }

    return physicalDevices;
}

void Instance::SelectPhysicalDevice(PhysicalDevice& device)
{
    if (device.Index >= mBackend->Devices.size())
    {
        throw std::runtime_error("Rendering::Systems::Instance:\nDesignated physical device does not exist");
    }

    mBackend->SelectedDevice = &mBackend->Devices[device.Index];
}

#endif