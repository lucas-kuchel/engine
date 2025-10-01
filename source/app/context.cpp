#include <app/context.hpp>

#include <stdexcept>

#include <GLFW/glfw3.h>

namespace app {
    Context::Context() {
        if (glfwInit() != GLFW_TRUE) {
            throw std::runtime_error("Call failed: app::Context::Context(): Failed to initialise window API");
        }

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    }

    Context::~Context() {
        glfwTerminate();
    }

    void Context::pollEvents() {
        glfwPollEvents();
    }

    void Context::awaitEvents() {
        glfwWaitEvents();
    }

    void Context::awaitEventsTimeout(double timeout) {
        glfwWaitEventsTimeout(timeout);
    }

    ContextBackend Context::queryBackend() const {
        switch (glfwGetPlatform()) {
            case GLFW_PLATFORM_COCOA:
                return ContextBackend::COCOA;

            case GLFW_PLATFORM_WAYLAND:
                return ContextBackend::WAYLAND;

            case GLFW_PLATFORM_WIN32:
                return ContextBackend::WIN32;

            case GLFW_PLATFORM_X11:
                return ContextBackend::X11;

            default:
                throw std::runtime_error("Call failed: app::GLFWContextImplementation::queryBackend(): Backend is unsupported");
        }
    }
}