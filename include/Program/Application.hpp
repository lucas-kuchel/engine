#pragma once

#include <Windowing/Systems/Instance.hpp>
#include <Windowing/Systems/Window.hpp>

#include <Rendering/Systems/Instance.hpp>

#include <string_view>
#include <vector>

namespace Program
{
    class Application
    {
    public:
        Application(const std::vector<std::string_view>& arguments);
        ~Application();

        void Update();

        bool ShouldUpdate() const;

    private:
        Windowing::Systems::Instance mWindowingInstance;
        Windowing::Systems::Window mWindow;

        Rendering::Systems::Instance mRenderingInstance;

        bool mShouldUpdate = true;

        Windowing::Systems::Window CreateWindow();
    };
}