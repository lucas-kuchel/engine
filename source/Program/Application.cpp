#include <Program/Application.hpp>

using namespace Program;

Application::Application(const std::vector<std::string_view>&)
    : mWindow(CreateWindow())
{
}

Application::~Application()
{
}

void Application::Update()
{
    mWindowingInstance.ProcessEvents();

    auto& events = mWindow.QueryEvents();

    while (!events.empty())
    {
        auto& event = events.front();

        if (event.Type == Windowing::Systems::WindowEventType::Closed)
        {
            mShouldUpdate = false;
        }

        events.pop();
    }
}

bool Application::ShouldUpdate() const
{
    return mShouldUpdate;
}

Windowing::Systems::Window Application::CreateWindow()
{
    Windowing::Systems::WindowDescriptor descriptor = {
        .Size = {800, 600},
        .Title = "New Window",
        .Visibility = Windowing::Systems::WindowVisibility::Windowed,
        .Show = true,
        .Resizable = false,
    };

    return mWindowingInstance.CreateWindow(descriptor);
}