#pragma once

#include <queue>
#include <string>

#include <glm/glm.hpp>

struct GLFWwindow;

namespace renderer {
    class Surface;
}

namespace app {
    enum class WindowVisibility : int;
    enum class WindowEventType : int;
    enum class MouseButton : int;
    enum class Key : int;

    class Context;

    // @brief Creation info for a window
    struct WindowCreateInfo {
        Context& context;

        glm::uvec2 extent;

        std::string title;

        WindowVisibility visibility;

        bool resizable;
    };

    struct WindowResizeEventInfo {
        glm::uvec2 extent;
    };

    struct WindowKeyPressedEventInfo {
        Key key;
    };

    struct WindowKeyReleasedEventInfo {
        Key key;
    };

    struct WindowMouseButtonPressedEventInfo {
        MouseButton button;
    };

    struct WindowMouseButtonReleasedEventInfo {
        MouseButton button;
    };

    struct WindowMouseScrolledEventInfo {
        glm::dvec2 offset;
    };

    struct WindowMouseMovedEventInfo {
        glm::dvec2 position;
    };

    union WindowEventInfo {
        WindowResizeEventInfo resize;
        WindowKeyPressedEventInfo keyPress;
        WindowKeyReleasedEventInfo keyRelease;
        WindowMouseButtonPressedEventInfo mouseButtonPress;
        WindowMouseButtonReleasedEventInfo mouseButtonRelease;
        WindowMouseScrolledEventInfo mouseScroll;
        WindowMouseMovedEventInfo mouseMove;
    };

    // @brief Event emitted by a window
    struct WindowEvent {
        WindowEventType type;
        WindowEventInfo info;
    };

    // @brief Generic platform-independent window
    // @note Not safe to copy
    class Window {
    public:
        Window(const WindowCreateInfo& createInfo);
        ~Window();

        Window(const Window&) = delete;
        Window(Window&&) noexcept = default;

        Window& operator=(const Window&) = delete;
        Window& operator=(Window&&) noexcept = default;

        // @brief Sets the window's extent
        // @param The requested window extent
        void setExtent(const glm::uvec2& extent);

        // @brief Sets the window's title
        // @param The requested title
        void setTitle(const std::string& title);

        // @brief Sets the window's visibility state
        // @param The requested visibility state
        void setVisibility(const WindowVisibility& visibility);

        // @brief Gets the window's extent
        // @return The extent of the window
        [[nodiscard]] const glm::uvec2& extent() const;

        // @brief Gets the window's title
        // @return The title of the window
        [[nodiscard]] const std::string& getTitle() const;

        // @brief Gets the window's visibility state
        // @return The visibility state of the window
        [[nodiscard]] const WindowVisibility& getVisibility() const;

        // @brief Returns if the window has events to process
        // @return If events need polling
        [[nodiscard]] bool hasEvents() const;

        // @brief Gets the window's next queued event
        // @return The next event of the window
        // @throws If no events are queued
        [[nodiscard]] WindowEvent getNextEvent();

        // @brief Gets the window's agnostic handle (GLFWwindow*)
        // @return The agnostic handle of the window
        [[nodiscard]] GLFWwindow*& getAgnosticHandle();

    private:
        glm::uvec2 extent_;
        glm::uvec2 lastWindowedExtent_;

        std::string title_;
        std::queue<WindowEvent> events_;

        WindowVisibility visibility_;

        GLFWwindow* handle_ = nullptr;

        static void resizeCallback(GLFWwindow* window, int width, int height);
        static void closeCallback(GLFWwindow* window);
        static void focusCallback(GLFWwindow* window, int focused);
        static void iconifyCallback(GLFWwindow* window, int iconified);
        static void keyCallback(GLFWwindow* window, int key, int, int action, int);
        static void mouseButtonCallback(GLFWwindow* window, int button, int action, int);
        static void mousePositionCallback(GLFWwindow* window, double x, double y);
        static void mouseScrollCallback(GLFWwindow* window, double x, double y);

        [[nodiscard]] static Key mapKey(int key);
        [[nodiscard]] static MouseButton mapMouseButton(int button);

        friend class renderer::Surface;
    };
}