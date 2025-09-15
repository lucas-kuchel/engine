#pragma once

#include <queue>
#include <string>

#include <glm/glm.hpp>

struct GLFWwindow;

namespace Windowing::Systems
{
    enum class WindowVisibility
    {
        Fullscreen,
        Windowed,
        Iconified,
    };

    enum class WindowEventType
    {
        Resized,
        Closed,
        Focused,
        Unfocused,
        Iconified,
        Restored,
        KeyPressed,
        KeyReleased,
        MouseMoved,
        MouseScrolled,
        MouseButtonPressed,
        MouseButtonReleased,
    };

    enum class Key
    {
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
        Space,
        LeftControl,
        RightControl,
        LeftSuper,
        RightSuper,
        LeftAlt,
        RightAlt,
        Capslock,
        Tab,
        Escape,
        LeftShift,
        RightShift,
        Home,
        End,
        PageUp,
        PageDown,
        Insert,
        Delete,
        Enter,
        Backspace,
        Tilde,
        Equal,
        Minus,
        NumpadPlus,
        NumpadMinus,
        Forwardslash,
        Backslash,
        Semicolon,
        Apostrophe,
        Comma,
        Period,
        Row0,
        Row1,
        Row2,
        Row3,
        Row4,
        Row5,
        Row6,
        Row7,
        Row8,
        Row9,
        Numpad0,
        Numpad1,
        Numpad2,
        Numpad3,
        Numpad4,
        Numpad5,
        Numpad6,
        Numpad7,
        Numpad8,
        Numpad9,
        UpArrow,
        DownArrow,
        LeftArrow,
        RightArrow
    };

    enum class MouseButton
    {
        Left,
        Right,
        Middle,
        Forward,
        Back
    };

    enum class CursorMode
    {
        Normal,
        Hidden,
        Disabled
    };

    struct WindowEvent
    {
        WindowEventType Type;

        struct ResizeData
        {
            glm::uvec2 Size;
        };

        struct KeyData
        {
            Key Keycode;
            std::size_t Scancode;
            std::size_t Modifiers;
        };

        struct MouseButtonData
        {
            MouseButton Button;
            std::size_t Modifiers;
        };

        struct MouseMoveData
        {
            glm::dvec2 Position;
            glm::dvec2 Delta;
        };

        struct MouseScrollData
        {
            glm::dvec2 Scroll;
            glm::dvec2 Delta;
        };

        union
        {
            ResizeData ResizeInfo;
            KeyData KeyInfo;
            MouseButtonData MouseButtonInfo;
            MouseMoveData MouseMoveInfo;
            MouseScrollData MouseScrollInfo;
        };
    };

    struct WindowDescriptor
    {
        glm::uvec2 Size;

        std::string Title;

        WindowVisibility Visibility;

        bool Show;
        bool Resizable;
    };

    class Window
    {
    public:
        ~Window();

        std::queue<WindowEvent>& QueryEvents();

        void SetVisibility(WindowVisibility visibility);
        void SetSize(const glm::uvec2& size);
        void SetTitle(const std::string& title);
        void SetCursorMode(CursorMode cursorMode);
        void SetShown(bool shown);

        const WindowVisibility& GetVisibility() const;
        const glm::uvec2& GetSize() const;
        const std::string& GetTitle() const;
        const CursorMode& GetCursorMode() const;

        const bool& IsShown() const;
        const bool& IsResizable() const;

        static constexpr std::size_t GetKeyCount();
        static constexpr std::size_t GetMouseButtonCount();

        void* GetHandle();

    private:
        Window(WindowDescriptor& descriptor);

        friend class Instance;

        GLFWwindow* mHandle;

        glm::dvec2 mLastMousePosition;
        glm::dvec2 mAccumulatedMouseScroll;

        WindowVisibility mVisibility;
        CursorMode mCursorMode;
        glm::uvec2 mSize;
        std::string mTitle;

        std::queue<WindowEvent> mEvents;

        bool mResizable;
        bool mShown;

        static void SizeCallback(GLFWwindow* window, int width, int height);
        static void CloseCallback(GLFWwindow* window);
        static void FocusCallback(GLFWwindow* window, int focused);
        static void IconifyCallback(GLFWwindow* window, int iconified);
        static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
        static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
        static void MousePositionCallback(GLFWwindow* window, double x, double y);
        static void MouseScrollCallback(GLFWwindow* window, double x, double y);
    };
}