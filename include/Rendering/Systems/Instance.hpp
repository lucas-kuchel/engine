#pragma once

#include <Rendering/Resources/PhysicalDevice.hpp>

#include <memory>
#include <vector>

namespace Rendering::Systems
{

    class Instance
    {
    public:
        Instance();
        ~Instance();

        std::vector<Resources::PhysicalDevice> QueryPhysicalDevices() const;

        void SelectPhysicalDevice(Resources::PhysicalDevice& physicalDevice);

    private:
        class Backend;

        std::unique_ptr<Backend> mBackend;
    };
}