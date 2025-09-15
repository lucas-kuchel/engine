#include <Windowing/Systems/Instance.hpp>

#include <stdexcept>

#include <GLFW/glfw3.h>

using namespace Windowing::Systems;

Instance::Instance()
{
    if (glfwInit() != GLFW_TRUE)
    {
        throw std::runtime_error("Window::Instance:\nFailed to create window instance");
    }
}

Instance::~Instance()
{
    glfwTerminate();
}

void Instance::ProcessEvents()
{
    glfwPollEvents();
}

void Instance::AwaitEvents()
{
    glfwWaitEvents();
}

void Instance::AwaitEventsTimeout(double timeout)
{
    glfwWaitEventsTimeout(timeout);
}

Window Instance::CreateWindow(WindowDescriptor& descriptor)
{
    return Window(descriptor);
}