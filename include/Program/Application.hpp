#pragma once

#include <Windowing/Systems/Instance.hpp>
#include <Windowing/Systems/Window.hpp>

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

        bool mShouldUpdate = true;

        Windowing::Systems::Window CreateWindow();
    };
}