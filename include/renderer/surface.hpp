#pragma once

#include <data/extent.hpp>
#include <data/references.hpp>

#include <cstdint>

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

    // @brief Represents a window's renderable area
    // @note Not safe to copy
    class Surface {
    public:
        Surface(const SurfaceCreateInfo& createInfo);
        ~Surface();

        Surface(const Surface&) = delete;
        Surface(Surface&&) noexcept = default;

        Surface& operator=(const Surface&) = delete;
        Surface& operator=(Surface&&) noexcept = default;

        // @brief Provides the extent of the surface
        // @return The extent of the surface
        [[nodiscard]] data::Extent2D<std::uint32_t> getExtent() const;

        // @brief Provides the VkSurface
        // @return The VkSurface
        [[nodiscard]] VkSurfaceKHR& getVkSurface();

        // @brief Provides the VkSurface
        // @return The VkSurface
        [[nodiscard]] const VkSurfaceKHR& getVkSurface() const;

    private:
        VkSurfaceKHR surface_ = VK_NULL_HANDLE;

        data::Reference<Instance> instance_;
        data::Reference<app::Window> window_;
    };
}