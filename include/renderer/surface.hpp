#pragma once

#include <vulkan/vulkan.h>

struct GLFWwindow;

namespace app {
    class Window;
}

namespace renderer {
    class Instance;

    struct SurfaceData {
        VkSurfaceKHR surface = VK_NULL_HANDLE;
        VkInstance instance = VK_NULL_HANDLE;
        app::Window* window = nullptr;
    };

    struct SurfaceCreateInfo {
        Instance& instance;
        app::Window& window;
    };

    class Surface {
    public:
        Surface();
        ~Surface();

        Surface(const Surface&) = delete;
        Surface(Surface&&) noexcept = default;

        Surface& operator=(const Surface&) = delete;
        Surface& operator=(Surface&&) noexcept = default;

        void create(const SurfaceCreateInfo& createInfo);

        SurfaceData& getData();

    private:
        SurfaceData data_;
    };
}