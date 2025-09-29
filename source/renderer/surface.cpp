#include <renderer/instance.hpp>
#include <renderer/surface.hpp>

#include <app/window.hpp>

#include <GLFW/glfw3.h>

namespace renderer {
    Surface::Surface(const SurfaceCreateInfo& createInfo)
        : instance_(createInfo.instance), window_(createInfo.window) {
        if (glfwCreateWindowSurface(instance_->getVkInstance(), window_->getAgnosticHandle(), nullptr, &surface_) != VK_SUCCESS) {
            throw std::runtime_error("Error calling renderer::Surface::create(): Failed to create window surface");
        }
    }

    Surface::~Surface() {
        if (surface_) {
            vkDestroySurfaceKHR(instance_->getVkInstance(), surface_, nullptr);

            surface_ = VK_NULL_HANDLE;
        }
    }

    data::Extent2D<std::uint32_t> Surface::getExtent() const {
        return window_->getExtent();
    }

    VkSurfaceKHR& Surface::getVkSurface() {
        return surface_;
    }

    const VkSurfaceKHR& Surface::getVkSurface() const {
        return surface_;
    }
}