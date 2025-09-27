#include <app/window.hpp>
#include <app/window/glfw.hpp>

#include <stdexcept>

namespace app {
    void Window::create(const WindowCreateInfo& createInfo) {
        if (implementation_) {
            throw std::runtime_error("Illegal call to app::Window::create(): Window already valid");
        }

        implementation_ = std::make_unique<GLFWWindowImplementation>(createInfo);
    }

    void Window::setExtent(const data::Extent2D& extent) {
        if (!implementation_) {
            throw std::runtime_error("Illegal call to app::Window::setExtent(): Window is invalid");
        }
        implementation_->setExtent(extent);
    }

    void Window::setTitle(const std::string& title) {
        if (!implementation_) {
            throw std::runtime_error("Illegal call to app::Window::setTitle(): Window is invalid");
        }

        implementation_->setTitle(title);
    }

    void Window::setVisibility(const WindowVisibility& visibility) {
        if (!implementation_) {
            throw std::runtime_error("Illegal call to app::Window::setVisibility(): Window is invalid");
        }

        implementation_->setVisibility(visibility);
    }

    const data::Extent2D& Window::getExtent() const {
        if (!implementation_) {
            throw std::runtime_error("Illegal call to app::Window::getExtent(): Window is invalid");
        }

        return implementation_->getExtent();
    }

    const std::string& Window::getTitle() const {
        if (!implementation_) {
            throw std::runtime_error("Illegal call to app::Window::getTitle(): Window is invalid");
        }

        return implementation_->getTitle();
    }

    const WindowVisibility& Window::getVisibility() const {
        if (!implementation_) {
            throw std::runtime_error("Illegal call to app::Window::getVisibility(): Window is invalid");
        }

        return implementation_->getVisibility();
    }

    std::queue<WindowEvent>& Window::queryEvents() {
        if (!implementation_) {
            throw std::runtime_error("Illegal call to app::Window::queryEvents(): Window is invalid");
        }

        return implementation_->queryEvents();
    }

    void* Window::getNativeHandle() {
        if (!implementation_) {
            throw std::runtime_error("Illegal call to app::Window::getNativeHandle(): Window is invalid");
        }

        return implementation_->getNativeHandle();
    }
}