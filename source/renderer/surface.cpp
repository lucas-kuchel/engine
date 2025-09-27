#include <renderer/instance.hpp>
#include <renderer/surface.hpp>

#include <app/window.hpp>

#include <GLFW/glfw3.h>

namespace renderer {
    Surface::Surface() {
    }

    Surface::~Surface() {
        if (data_.surface) {
            vkDestroySurfaceKHR(data_.instance, data_.surface, nullptr);

            data_.instance = VK_NULL_HANDLE;
            data_.surface = VK_NULL_HANDLE;
            data_.window = nullptr;
        }
    }

    void Surface::create(const SurfaceCreateInfo& createInfo) {
        data_.window = &createInfo.window;
        data_.instance = createInfo.instance.getData().instance;

        if (glfwCreateWindowSurface(data_.instance, data_.window->getAgnosticHandle(), nullptr, &data_.surface) != VK_SUCCESS) {
            throw std::runtime_error("Error calling renderer::Surface::create(): Failed to create window surface");
        }
    }

    SurfaceData& Surface::getData() {
        return data_;
    }
}