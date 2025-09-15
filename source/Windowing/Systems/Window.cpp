#include <Windowing/Systems/Window.hpp>

#include <stdexcept>

#include <GLFW/glfw3.h>

using namespace Windowing::Systems;

Key MapKey(int glfwKey)
{
    switch (glfwKey)
    {
        case GLFW_KEY_A:
        {
            return Key::A;
        }
        case GLFW_KEY_B:
        {
            return Key::B;
        }
        case GLFW_KEY_C:
        {
            return Key::C;
        }
        case GLFW_KEY_D:
        {
            return Key::D;
        }
        case GLFW_KEY_E:
        {
            return Key::E;
        }
        case GLFW_KEY_F:
        {
            return Key::F;
        }
        case GLFW_KEY_G:
        {
            return Key::G;
        }
        case GLFW_KEY_H:
        {
            return Key::H;
        }
        case GLFW_KEY_I:
        {
            return Key::I;
        }
        case GLFW_KEY_J:
        {
            return Key::J;
        }
        case GLFW_KEY_K:
        {
            return Key::K;
        }
        case GLFW_KEY_L:
        {
            return Key::L;
        }
        case GLFW_KEY_M:
        {
            return Key::M;
        }
        case GLFW_KEY_N:
        {
            return Key::N;
        }
        case GLFW_KEY_O:
        {
            return Key::O;
        }
        case GLFW_KEY_P:
        {
            return Key::P;
        }
        case GLFW_KEY_Q:
        {
            return Key::Q;
        }
        case GLFW_KEY_R:
        {
            return Key::R;
        }
        case GLFW_KEY_S:
        {
            return Key::S;
        }
        case GLFW_KEY_T:
        {
            return Key::T;
        }
        case GLFW_KEY_U:
        {
            return Key::U;
        }
        case GLFW_KEY_V:
        {
            return Key::V;
        }
        case GLFW_KEY_W:
        {
            return Key::W;
        }
        case GLFW_KEY_X:
        {
            return Key::X;
        }
        case GLFW_KEY_Y:
        {
            return Key::Y;
        }
        case GLFW_KEY_Z:
        {
            return Key::Z;
        }
        case GLFW_KEY_F1:
        {
            return Key::F1;
        }
        case GLFW_KEY_F2:
        {
            return Key::F2;
        }
        case GLFW_KEY_F3:
        {
            return Key::F3;
        }
        case GLFW_KEY_F4:
        {
            return Key::F4;
        }
        case GLFW_KEY_F5:
        {
            return Key::F5;
        }
        case GLFW_KEY_F6:
        {
            return Key::F6;
        }
        case GLFW_KEY_F7:
        {
            return Key::F7;
        }
        case GLFW_KEY_F8:
        {
            return Key::F8;
        }
        case GLFW_KEY_F9:
        {
            return Key::F9;
        }
        case GLFW_KEY_F10:
        {
            return Key::F10;
        }
        case GLFW_KEY_F11:
        {
            return Key::F11;
        }
        case GLFW_KEY_F12:
        {
            return Key::F12;
        }
        case GLFW_KEY_SPACE:
        {
            return Key::Space;
        }
        case GLFW_KEY_LEFT_CONTROL:
        {
            return Key::LeftControl;
        }
        case GLFW_KEY_RIGHT_CONTROL:
        {
            return Key::RightControl;
        }
        case GLFW_KEY_LEFT_SUPER:
        {
            return Key::LeftSuper;
        }
        case GLFW_KEY_RIGHT_SUPER:
        {
            return Key::RightSuper;
        }
        case GLFW_KEY_LEFT_ALT:
        {
            return Key::LeftAlt;
        }
        case GLFW_KEY_RIGHT_ALT:
        {
            return Key::RightAlt;
        }
        case GLFW_KEY_CAPS_LOCK:
        {
            return Key::Capslock;
        }
        case GLFW_KEY_TAB:
        {
            return Key::Tab;
        }
        case GLFW_KEY_ESCAPE:
        {
            return Key::Escape;
        }
        case GLFW_KEY_LEFT_SHIFT:
        {
            return Key::LeftShift;
        }
        case GLFW_KEY_RIGHT_SHIFT:
        {
            return Key::RightShift;
        }
        case GLFW_KEY_HOME:
        {
            return Key::Home;
        }
        case GLFW_KEY_END:
        {
            return Key::End;
        }
        case GLFW_KEY_PAGE_UP:
        {
            return Key::PageUp;
        }
        case GLFW_KEY_PAGE_DOWN:
        {
            return Key::PageDown;
        }
        case GLFW_KEY_INSERT:
        {
            return Key::Insert;
        }
        case GLFW_KEY_DELETE:
        {
            return Key::Delete;
        }
        case GLFW_KEY_ENTER:
        {
            return Key::Enter;
        }
        case GLFW_KEY_BACKSPACE:
        {
            return Key::Backspace;
        }
        case GLFW_KEY_GRAVE_ACCENT:
        {
            return Key::Tilde;
        }
        case GLFW_KEY_EQUAL:
        {
            return Key::Equal;
        }
        case GLFW_KEY_MINUS:
        {
            return Key::Minus;
        }
        case GLFW_KEY_SLASH:
        {
            return Key::Forwardslash;
        }
        case GLFW_KEY_BACKSLASH:
        {
            return Key::Backslash;
        }
        case GLFW_KEY_SEMICOLON:
        {
            return Key::Semicolon;
        }
        case GLFW_KEY_APOSTROPHE:
        {
            return Key::Apostrophe;
        }
        case GLFW_KEY_COMMA:
        {
            return Key::Comma;
        }
        case GLFW_KEY_PERIOD:
        {
            return Key::Period;
        }
        case GLFW_KEY_0:
        {
            return Key::Row0;
        }
        case GLFW_KEY_1:
        {
            return Key::Row1;
        }
        case GLFW_KEY_2:
        {
            return Key::Row2;
        }
        case GLFW_KEY_3:
        {
            return Key::Row3;
        }
        case GLFW_KEY_4:
        {
            return Key::Row4;
        }
        case GLFW_KEY_5:
        {
            return Key::Row5;
        }
        case GLFW_KEY_6:
        {
            return Key::Row6;
        }
        case GLFW_KEY_7:
        {
            return Key::Row7;
        }
        case GLFW_KEY_8:
        {
            return Key::Row8;
        }
        case GLFW_KEY_9:
        {
            return Key::Row9;
        }
        case GLFW_KEY_KP_ADD:
        {
            return Key::NumpadPlus;
        }
        case GLFW_KEY_KP_SUBTRACT:
        {
            return Key::NumpadMinus;
        }
        case GLFW_KEY_KP_0:
        {
            return Key::Numpad0;
        }
        case GLFW_KEY_KP_1:
        {
            return Key::Numpad1;
        }
        case GLFW_KEY_KP_2:
        {
            return Key::Numpad2;
        }
        case GLFW_KEY_KP_3:
        {
            return Key::Numpad3;
        }
        case GLFW_KEY_KP_4:
        {
            return Key::Numpad4;
        }
        case GLFW_KEY_KP_5:
        {
            return Key::Numpad5;
        }
        case GLFW_KEY_KP_6:
        {
            return Key::Numpad6;
        }
        case GLFW_KEY_KP_7:
        {
            return Key::Numpad7;
        }
        case GLFW_KEY_KP_8:
        {
            return Key::Numpad8;
        }
        case GLFW_KEY_KP_9:
        {
            return Key::Numpad9;
        }
        case GLFW_KEY_UP:
        {
            return Key::UpArrow;
        }
        case GLFW_KEY_DOWN:
        {
            return Key::DownArrow;
        }
        case GLFW_KEY_LEFT:
        {
            return Key::LeftArrow;
        }
        case GLFW_KEY_RIGHT:
        {
            return Key::RightArrow;
        }
    }

    throw;
}

MouseButton MapButton(int glfwButton)
{
    switch (glfwButton)
    {
        case GLFW_MOUSE_BUTTON_LEFT:
        {
            return MouseButton::Left;
        }
        case GLFW_MOUSE_BUTTON_RIGHT:
        {
            return MouseButton::Right;
        }
        case GLFW_MOUSE_BUTTON_MIDDLE:
        {
            return MouseButton::Middle;
        }
        case GLFW_MOUSE_BUTTON_4:
        {
            return MouseButton::Forward;
        }
        case GLFW_MOUSE_BUTTON_5:
        {
            return MouseButton::Back;
        }
    }

    throw;
}

Window::Window(WindowDescriptor& descriptor)
    : mVisibility(descriptor.Visibility), mSize(descriptor.Size), mTitle(descriptor.Title), mResizable(descriptor.Resizable), mShown(descriptor.Show)
{
    glfwWindowHint(GLFW_RESIZABLE, mResizable ? GLFW_TRUE : GLFW_FALSE);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    mHandle = glfwCreateWindow(mSize.x, mSize.y, mTitle.c_str(), nullptr, nullptr);

    if (mHandle == nullptr)
    {
        throw std::runtime_error("Windowing::Systems::Window:\nFailed to create window");
    }

    if (!mShown)
    {
        glfwHideWindow(mHandle);
    }

    glfwSetWindowUserPointer(mHandle, this);

    glfwSetWindowSizeCallback(mHandle, SizeCallback);
    glfwSetWindowCloseCallback(mHandle, CloseCallback);
    glfwSetWindowFocusCallback(mHandle, FocusCallback);
    glfwSetWindowIconifyCallback(mHandle, IconifyCallback);
    glfwSetKeyCallback(mHandle, KeyCallback);
    glfwSetMouseButtonCallback(mHandle, MouseButtonCallback);
    glfwSetCursorPosCallback(mHandle, MousePositionCallback);
    glfwSetScrollCallback(mHandle, MouseScrollCallback);

    SetVisibility(mVisibility);
}

Window::~Window()
{
    glfwDestroyWindow(mHandle);
}

void Window::SetShown(bool shown)
{
    shown ? glfwShowWindow(mHandle) : glfwHideWindow(mHandle);
}

std::queue<WindowEvent>& Window::QueryEvents()
{
    return mEvents;
}

void Window::SetVisibility(WindowVisibility visibility)
{
    mVisibility = visibility;

    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);

    switch (mVisibility)
    {
        case WindowVisibility::Fullscreen:
        {
            glfwSetWindowMonitor(mHandle, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
            break;
        }
        case WindowVisibility::Iconified:
        {
            glfwIconifyWindow(mHandle);
            break;
        }
        default:
        {
            // Center the window on the primary monitor
            int monitorX, monitorY;
            glfwGetMonitorPos(monitor, &monitorX, &monitorY);

            int xpos = monitorX + (mode->width - static_cast<int>(mSize.x)) / 2;
            int ypos = monitorY + (mode->height - static_cast<int>(mSize.y)) / 2;

            glfwSetWindowMonitor(mHandle, nullptr, xpos, ypos, mSize.x, mSize.y, 0);
            break;
        }
    }
}

void Window::SetSize(const glm::uvec2& size)
{
    mSize = size;

    glfwSetWindowSize(mHandle, size.x, size.y);
}

void Window::SetTitle(const std::string& title)
{
    mTitle = title;

    glfwSetWindowTitle(mHandle, mTitle.c_str());
}

const WindowVisibility& Window::GetVisibility() const
{
    return mVisibility;
}

const glm::uvec2& Window::GetSize() const
{
    return mSize;
}

const std::string& Window::GetTitle() const
{
    return mTitle;
}

const bool& Window::IsShown() const
{
    return mShown;
}

const bool& Window::IsResizable() const
{
    return mResizable;
}

void Window::SetCursorMode(CursorMode cursorMode)
{
    int glfwMode = GLFW_CURSOR_NORMAL;

    switch (cursorMode)
    {
        case CursorMode::Normal:
        {
            glfwMode = GLFW_CURSOR_NORMAL;

            break;
        }
        case CursorMode::Hidden:
        {
            glfwMode = GLFW_CURSOR_HIDDEN;

            break;
        }
        case CursorMode::Disabled:
        {
            glfwMode = GLFW_CURSOR_DISABLED;

            break;
        }
    }

    glfwSetInputMode(mHandle, GLFW_CURSOR, glfwMode);

    mCursorMode = cursorMode;
}

constexpr std::size_t Window::GetKeyCount()
{
    return 93;
}

constexpr std::size_t Window::GetMouseButtonCount()
{
    return 5;
}

const CursorMode& Window::GetCursorMode() const
{
    return mCursorMode;
}

void* Window::GetHandle()
{
    return mHandle;
}

void Window::SizeCallback(GLFWwindow* window, int width, int height)
{
    auto* self = static_cast<Window*>(glfwGetWindowUserPointer(window));

    if (!self)
    {
        return;
    }

    WindowEvent event;

    event.Type = WindowEventType::Resized;

    event.ResizeInfo.Size = {width, height};

    self->mSize = event.ResizeInfo.Size;
    self->mEvents.push(event);
}

void Window::CloseCallback(GLFWwindow* window)
{
    auto* self = static_cast<Window*>(glfwGetWindowUserPointer(window));

    if (!self)
    {
        return;
    }

    WindowEvent event;

    event.Type = WindowEventType::Closed;

    self->mEvents.push(event);
}

void Window::FocusCallback(GLFWwindow* window, int focused)
{
    auto* self = static_cast<Window*>(glfwGetWindowUserPointer(window));

    if (!self)
    {
        return;
    }

    WindowEvent event;

    event.Type = focused ? WindowEventType::Focused : WindowEventType::Unfocused;

    self->mEvents.push(event);
}

void Window::IconifyCallback(GLFWwindow* window, int iconified)
{
    auto* self = static_cast<Window*>(glfwGetWindowUserPointer(window));

    if (!self)
    {
        return;
    }

    WindowEvent event;

    event.Type = iconified ? WindowEventType::Iconified : WindowEventType::Restored;

    self->mEvents.push(event);
    self->mVisibility = iconified ? WindowVisibility::Iconified : WindowVisibility::Windowed;
}

void Window::KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    auto* self = static_cast<Window*>(glfwGetWindowUserPointer(window));

    if (!self)
    {
        return;
    }

    if (action != GLFW_PRESS && action != GLFW_RELEASE)
    {
        return;
    }

    WindowEvent event;

    event.KeyInfo.Keycode = MapKey(key);

    event.Type = action == GLFW_PRESS ? WindowEventType::KeyPressed : WindowEventType::KeyReleased;
    event.KeyInfo.Scancode = scancode;
    event.KeyInfo.Modifiers = mods;

    self->mEvents.push(event);
}

void Window::MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    auto* self = static_cast<Window*>(glfwGetWindowUserPointer(window));

    if (!self)
    {
        return;
    }

    if (action != GLFW_PRESS && action != GLFW_RELEASE)
    {
        return;
    }

    WindowEvent event;

    event.MouseButtonInfo.Button = MapButton(button);
    event.Type = action == GLFW_PRESS ? WindowEventType::MouseButtonPressed : WindowEventType::MouseButtonReleased;
    event.MouseButtonInfo.Modifiers = mods;

    self->mEvents.push(event);
}

void Window::MousePositionCallback(GLFWwindow* window, double x, double y)
{
    auto* self = static_cast<Window*>(glfwGetWindowUserPointer(window));

    if (!self)
    {
        return;
    }

    WindowEvent event;

    event.Type = WindowEventType::MouseMoved;
    event.MouseMoveInfo.Delta = glm::dvec2(x, y) - self->mLastMousePosition;
    event.MouseMoveInfo.Position = {x, y};

    self->mLastMousePosition = {x, y};
    self->mEvents.push(event);
}

void Window::MouseScrollCallback(GLFWwindow* window, double x, double y)
{
    auto* self = static_cast<Window*>(glfwGetWindowUserPointer(window));

    if (!self)
    {
        return;
    }

    WindowEvent event;
    event.Type = WindowEventType::MouseScrolled;

    event.MouseScrollInfo.Delta = {x, y};
    self->mAccumulatedMouseScroll += event.MouseScrollInfo.Delta;
    event.MouseScrollInfo.Scroll = self->mAccumulatedMouseScroll;

    self->mEvents.push(event);
}