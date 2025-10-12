#include <renderer/instance.hpp>
#include <renderer/surface.hpp>

#include <app/window.hpp>

#include <GLFW/glfw3.h>

namespace renderer {
    Surface Surface::create(const SurfaceCreateInfo& createInfo) {
        Surface surface;

        if (glfwCreateWindowSurface(createInfo.instance.instance_, createInfo.window.handle_, nullptr, &surface.surface_) != VK_SUCCESS) {
            surface.surface_ = nullptr;
        }
        else {
            surface.window_ = &createInfo.window;
            surface.instance_ = &createInfo.instance;
        }

        return surface;
    }

    void Surface::destroy(Surface& surface) {
        if (surface.surface_) {
            vkDestroySurfaceKHR(surface.instance_->instance_, surface.surface_, nullptr);

            surface.surface_ = nullptr;
        }
    }

    glm::uvec2 Surface::extent(Surface& surface) {
        return surface.window_->extent();
    }
}