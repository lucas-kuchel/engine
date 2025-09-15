#pragma once

#include <Windowing/Systems/Window.hpp>

namespace Windowing::Systems
{
    class Instance
    {
    public:
        Instance();
        ~Instance();

        void ProcessEvents();
        void AwaitEvents();
        void AwaitEventsTimeout(double timeout);

        Window CreateWindow(WindowDescriptor& descriptor);
    };
}