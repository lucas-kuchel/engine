#pragma once

#include <data/extent.hpp>

#include <app/events.hpp>

#include <memory>
#include <queue>
#include <string>

namespace app {
    class Context;

    // @brief Visibility state of a window
    enum class WindowVisibility {
        FULLSCREEN,
        WINDOWED,
        MINIMISED,
    };

    // @brief Creation info for a window
    struct WindowCreateInfo {
        Context& context;

        data::Extent2D extent;

        std::string title;

        WindowVisibility visibility;

        bool resizable;
    };

    // @brief Generic platform-independent window
    // @note Not safe to copy
    class Window {
    public:
        Window() = default;
        ~Window() = default;

        Window(const Window&) = delete;
        Window(Window&&) noexcept = default;

        Window& operator=(const Window&) = delete;
        Window& operator=(Window&&) noexcept = default;

        // @brief Creates the window
        // @param The window create info
        // @throws std::runtime_error if window creation fails
        void create(const WindowCreateInfo& createInfo);

        // @brief Sets the window's extent
        // @param The requested window extent
        // @throws std::runtime_error if the window is in an invalid state
        void setExtent(const data::Extent2D& extent);

        // @brief Sets the window's title
        // @param The requested title
        // @throws std::runtime_error if the window is in an invalid state
        void setTitle(const std::string& title);

        // @brief Sets the window's visibility state
        // @param The requested visibility state
        // @throws std::runtime_error if the window is in an invalid state
        void setVisibility(const WindowVisibility& visibility);

        // @brief Gets the window's extent
        // @return The extent of the window
        // @throws std::runtime_error if the window is in an invalid state
        [[nodiscard]] const data::Extent2D& getExtent() const;

        // @brief Gets the window's title
        // @return The title of the window
        // @throws std::runtime_error if the window is in an invalid state
        [[nodiscard]] const std::string& getTitle() const;

        // @brief Gets the window's visibility state
        // @return The visibility state of the window
        // @throws std::runtime_error if the window is in an invalid state
        [[nodiscard]] const WindowVisibility& getVisibility() const;

        // @brief Gets the window's queued events
        // @return The queued events of the window
        // @throws std::runtime_error if the window is in an invalid state
        [[nodiscard]] std::queue<WindowEvent>& queryEvents();

        // @brief Gets the window's native platform handle
        // @return The native handle of the window for the current platform
        // @note Gives HWND* on Windows, NSWindow* on macOS and either a Window* or wl_surface* on Linux
        // @throws std::runtime_error if the window is in an invalid state
        void* getNativeHandle();

    private:
        class Implementation {
        public:
            Implementation() = default;
            virtual ~Implementation() = default;

            virtual void setExtent(const data::Extent2D& extent) = 0;
            virtual void setTitle(const std::string& title) = 0;
            virtual void setVisibility(const WindowVisibility& visibility) = 0;

            [[nodiscard]] virtual const data::Extent2D& getExtent() const = 0;
            [[nodiscard]] virtual const std::string& getTitle() const = 0;
            [[nodiscard]] virtual const WindowVisibility& getVisibility() const = 0;
            [[nodiscard]] virtual std::queue<WindowEvent>& queryEvents() = 0;

            virtual void* getNativeHandle() = 0;
        };

        std::unique_ptr<Implementation> implementation_;

        friend class GLFWWindowImplementation;
    };
}