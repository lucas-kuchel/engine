#include <app/context/glfw.hpp>

#include <stdexcept>

#include <GLFW/glfw3.h>

namespace app {
    GLFWContextImplementation::GLFWContextImplementation() {
        if (glfwInit() != GLFW_TRUE) {
            throw std::runtime_error("Error calling app::GLFWContextImplementation::GLFWContextImplementation(): Failed to initialise GLFW");
        }

        initialised_ = true;

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    }

    GLFWContextImplementation::~GLFWContextImplementation() {
        if (initialised_) {
            glfwTerminate();

            initialised_ = false;
        }
    }

    void GLFWContextImplementation::pollEvents() {
        glfwPollEvents();
    }

    void GLFWContextImplementation::awaitEvents() {
        glfwWaitEvents();
    }

    void GLFWContextImplementation::awaitEventsTimeout(double timeout) {
        glfwWaitEventsTimeout(timeout);
    }

    ContextBackend GLFWContextImplementation::queryBackend() const {
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
                throw std::runtime_error("Error calling app::GLFWContextImplementation::queryBackend(): Backend is unsupported");
        }
    }
}