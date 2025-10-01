#pragma once

#include <data/extent.hpp>
#include <data/position.hpp>

#include <cstdint>
#include <queue>
#include <string>

struct GLFWwindow;

namespace app {
    // @brief Visibility state of a window
    enum class WindowVisibility {
        FULLSCREEN,
        WINDOWED,
        MINIMISED,
    };

    class Context;

    // @brief Creation info for a window
    struct WindowCreateInfo {
        Context& context;

        data::Extent2D<std::uint32_t> extent;

        std::string title;

        WindowVisibility visibility;

        bool resizable;
    };

    // @brief Event type for a window
    enum class WindowEventType {
        RESIZED,
        CLOSED,
        FOCUSED,
        UNFOCUSED,
        MINIMIZED,
        MAXIMIZED,
        RESTORED,
        ENTERED_FULLSCREEN,
        EXITED_FULLSCREEN,
        KEY_PRESSED,
        KEY_RELEASED,
        MOUSE_BUTTON_PRESSED,
        MOUSE_BUTTON_RELEASED,
        MOUSE_SCROLLED,
        MOUSE_MOVED,
    };

    // @brief Key relevant to an event
    enum class Key {
        A,
        B,
        C,
        D,
        E,
        F,
        G,
        H,
        I,
        J,
        K,
        L,
        M,
        N,
        O,
        P,
        Q,
        R,
        S,
        T,
        U,
        V,
        W,
        X,
        Y,
        Z,
        F1,
        F2,
        F3,
        F4,
        F5,
        F6,
        F7,
        F8,
        F9,
        F10,
        F11,
        F12,
        SPACE,
        LCTRL,
        RCTRL,
        LSUPER,
        RSUPER,
        LALT,
        RALT,
        CAPSLOCK,
        TAB,
        ESCAPE,
        LSHIFT,
        RSHIFT,
        HOME,
        END,
        PGUP,
        PGDN,
        INSERT,
        DELETE,
        ENTER,
        BACKSPACE,
        GRAVE_ACCENT,
        EQUAL,
        MINUS,
        KEYPAD_PLUS,
        KEYPAD_MINUS,
        SLASH,
        BACKSLASH,
        SEMICOLON,
        APOSTROPHE,
        COMMA,
        PERIOD,
        ROW_0,
        ROW_1,
        ROW_2,
        ROW_3,
        ROW_4,
        ROW_5,
        ROW_6,
        ROW_7,
        ROW_8,
        ROW_9,
        KEYPAD_0,
        KEYPAD_1,
        KEYPAD_2,
        KEYPAD_3,
        KEYPAD_4,
        KEYPAD_5,
        KEYPAD_6,
        KEYPAD_7,
        KEYPAD_8,
        KEYPAD_9,
        UP,
        DOWN,
        LEFT,
        RIGHT,
    };

    // @brief Mouse button relevant to an event
    enum class MouseButton {
        LEFT,
        RIGHT,
        MIDDLE,
    };

    struct WindowResizeEventInfo {
        data::Extent2D<std::uint32_t> extent;
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
        data::Position2D<double> offset;
    };

    struct WindowMouseMovedEventInfo {
        data::Position2D<double> position;
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
        void setExtent(const data::Extent2D<std::uint32_t>& extent);

        // @brief Sets the window's title
        // @param The requested title
        void setTitle(const std::string& title);

        // @brief Sets the window's visibility state
        // @param The requested visibility state
        void setVisibility(const WindowVisibility& visibility);

        // @brief Gets the window's extent
        // @return The extent of the window
        [[nodiscard]] const data::Extent2D<std::uint32_t>& getExtent() const;

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
        data::Extent2D<std::uint32_t> extent_;
        data::Extent2D<std::uint32_t> lastWindowedExtent_;

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
    };
}