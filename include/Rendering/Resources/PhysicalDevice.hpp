#pragma once

#include <cstdlib>
#include <string>

namespace Rendering::Resources
{
    enum class PhysicalDeviceType
    {
        Discrete,
        Integrated,
        Software,
    };

    struct PhysicalDevice
    {
        PhysicalDeviceType Type;

        std::string Name;

        std::size_t VendorID;
        std::size_t DeviceID;
        std::size_t Index;

        bool UnifiedMemory;
        bool SupportsRaytracing;
    };
}