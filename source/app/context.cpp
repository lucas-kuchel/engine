#include <app/context.hpp>
#include <app/context/glfw.hpp>

#include <stdexcept>

namespace app {
    Context::Context() {
        implementation_ = std::make_unique<GLFWContextImplementation>();
    }

    void Context::pollEvents() {
        if (!implementation_) {
            throw std::runtime_error("Illegal call to app::Context::pollEvents(): context is invalid");
        }

        implementation_->pollEvents();
    }

    void Context::awaitEvents() {
        if (!implementation_) {
            throw std::runtime_error("Illegal call to app::Context::awaitEvents(): context is invalid");
        }

        implementation_->awaitEvents();
    }

    void Context::awaitEventsTimeout(double timeout) {
        if (!implementation_) {
            throw std::runtime_error("Illegal call to app::Context::awaitEventsTimeout(): context is invalid");
        }

        implementation_->awaitEventsTimeout(timeout);
    }

    ContextBackend Context::queryBackend() const {
        if (!implementation_) {
            throw std::runtime_error("Illegal call to app::Context::queryBackend(): context is invalid");
        }

        return implementation_->queryBackend();
    }
}