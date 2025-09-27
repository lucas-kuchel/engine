#pragma once

#include <variant>

#include <data/extent.hpp>
#include <data/position.hpp>

namespace app {
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

    enum class MouseButton {
        LEFT,
        RIGHT,
        MIDDLE,
    };

    struct WindowResizeEventInfo {
        data::Extent2D extent;
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
        data::Position2D offset;
    };

    struct WindowMouseMovedEventInfo {
        data::Position2D position;
    };

    using WindowEventInfo = std::variant<
        std::monostate,
        WindowResizeEventInfo,
        WindowKeyPressedEventInfo,
        WindowKeyReleasedEventInfo,
        WindowMouseButtonPressedEventInfo,
        WindowMouseButtonReleasedEventInfo,
        WindowMouseScrolledEventInfo,
        WindowMouseMovedEventInfo>;

    struct WindowEvent {
        WindowEventType type;
        WindowEventInfo info;
    };
}