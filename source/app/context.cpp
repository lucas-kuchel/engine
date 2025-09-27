#include <app/context.hpp>

#include <stdexcept>

#include <GLFW/glfw3.h>

namespace app {
    class Context::Implementation {
    public:
        Implementation() {
            if (glfwInit() != GLFW_TRUE) {
                throw std::runtime_error("Error calling app::Context::Context(): Failed to initialise window API");
            }

            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        }

        ~Implementation() {
            glfwTerminate();
        }

        void pollEvents() {
            glfwPollEvents();
        }

        void awaitEvents() {
            glfwWaitEvents();
        }

        void awaitEventsTimeout(double timeout) {
            glfwWaitEventsTimeout(timeout);
        }

        ContextBackend queryBackend() const {
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

        const char** enumerateRequiredInstanceExtensions(std::uint32_t* count) {
            return glfwGetRequiredInstanceExtensions(count);
        }
    };

    Context::Context() {
        implementation_ = std::make_unique<Implementation>();
    }

    Context::~Context() = default;

    void Context::pollEvents() {
        if (!implementation_) {
            throw std::runtime_error("Illegal call to app::Context::pollEvents(): Context is invalid");
        }

        implementation_->pollEvents();
    }

    void Context::awaitEvents() {
        if (!implementation_) {
            throw std::runtime_error("Illegal call to app::Context::awaitEvents(): Context is invalid");
        }

        implementation_->awaitEvents();
    }

    void Context::awaitEventsTimeout(double timeout) {
        if (!implementation_) {
            throw std::runtime_error("Illegal call to app::Context::awaitEventsTimeout(): Context is invalid");
        }

        implementation_->awaitEventsTimeout(timeout);
    }

    ContextBackend Context::queryBackend() const {
        if (!implementation_) {
            throw std::runtime_error("Illegal call to app::Context::queryBackend(): Context is invalid");
        }

        return implementation_->queryBackend();
    }

    const char** Context::enumerateRequiredInstanceExtensions(std::uint32_t* count) {
        if (!implementation_) {
            throw std::runtime_error("Illegal call to app::Context::enumerateRequiredInstanceExtensions(): Context is invalid");
        }

        return implementation_->enumerateRequiredInstanceExtensions(count);
    }
}