#pragma once

#include <glm/glm.hpp>

#include <vulkan/vulkan.h>

struct GLFWwindow;

namespace app {
    class Window;
}

namespace renderer {
    class Instance;

    // @brief Creation information for a surface
    struct SurfaceCreateInfo {
        Instance& instance;
        app::Window& window;
    };

    class Surface {
    public:
        static Surface create(const SurfaceCreateInfo& createInfo);
        static void destroy(Surface& surface);

        static glm::uvec2 extent(Surface& surface);

    private:
        VkSurfaceKHR surface_ = nullptr;
        Instance* instance_ = nullptr;
        app::Window* window_ = nullptr;

        friend class Device;
        friend class Swapchain;
    };
}