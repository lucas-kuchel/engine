#if defined(PLATFORM_APPLE) && defined(PLATFORM_UNIX) && !defined(PLATFORM_WINDOWS)

#include <Rendering/Systems/Instance.hpp>
#include <Windowing/Systems/Window.hpp>

#import <Cocoa/Cocoa.h>
#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>

using namespace Rendering::Systems;
using namespace Rendering::Resources;

class Instance::Backend
{
public:
    id<MTLDevice> SelectedDevice = nil;
    NSArray<id<MTLDevice>>* AvailableDevices = nil;
};

Instance::Instance()
    : mBackend(std::make_unique<Backend>())
{
}

Instance::~Instance()
{
}

std::vector<PhysicalDevice> Instance::QueryPhysicalDevices() const
{
    mBackend->AvailableDevices = MTLCopyAllDevices();

    std::vector<PhysicalDevice> physicalDevices(mBackend->AvailableDevices.count);

    for (std::size_t i = 0; i < physicalDevices.size(); i++)
    {
        id<MTLDevice> metalDevice = [mBackend->AvailableDevices objectAtIndex:i];
        PhysicalDevice& physicalDevice = physicalDevices[i];

        physicalDevice.Name = [metalDevice.name UTF8String];
        physicalDevice.UnifiedMemory = metalDevice.hasUnifiedMemory;
        physicalDevice.SupportsRaytracing = metalDevice.supportsRaytracing;
        physicalDevice.Index = i;
        physicalDevice.VendorID = 0;
        physicalDevice.DeviceID = static_cast<std::size_t>(metalDevice.registryID);

        if (metalDevice.isRemovable)
        {
            physicalDevice.Type = PhysicalDeviceType::Discrete;
        }
        else
        {
            physicalDevice.Type = PhysicalDeviceType::Integrated;
        }
    }

    return physicalDevices;
}

void Instance::SelectPhysicalDevice(PhysicalDevice& device)
{
    if (mBackend->SelectedDevice != nil)
    {
        throw std::runtime_error("Rendering::Systems::Instance:\nPhysical device already selected");
    }
    else if (device.Index >= mBackend->AvailableDevices.count)
    {
        throw std::runtime_error("Rendering::Systems::Instance:\nDesignated physical device does not exist");
    }

    mBackend->SelectedDevice = [mBackend->AvailableDevices objectAtIndex:device.Index];

    [mBackend->AvailableDevices release];
}

#endif