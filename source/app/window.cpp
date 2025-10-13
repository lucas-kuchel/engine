#include <app/configuration.hpp>
#include <app/window.hpp>

#include <stdexcept>

#include <GLFW/glfw3.h>

namespace app {
    Window::Window(const WindowCreateInfo& createInfo)
        : extent_(createInfo.extent), title_(createInfo.title), visibility_(createInfo.visibility) {
        glfwWindowHint(GLFW_RESIZABLE, createInfo.resizable);

        handle_ = glfwCreateWindow(static_cast<std::int32_t>(extent_.x), static_cast<std::int32_t>(extent_.y), title_.c_str(), nullptr, nullptr);

        if (!handle_) {
            throw std::runtime_error("Call failed: app::Window::Window(): Failed to create window");
        }

        switch (visibility_) {
            case WindowVisibility::WINDOWED:
                glfwRestoreWindow(handle_);
                break;

            case WindowVisibility::MINIMISED:
                glfwIconifyWindow(handle_);
                break;

            case WindowVisibility::FULLSCREEN: {
                GLFWmonitor* monitor = glfwGetPrimaryMonitor();
                const GLFWvidmode* mode = glfwGetVideoMode(monitor);

                glfwSetWindowMonitor(
                    handle_,
                    monitor,
                    0, 0,
                    mode->width, mode->height,
                    mode->refreshRate);
                break;
            }
        }

        glfwSetWindowUserPointer(handle_, this);

        glfwSetWindowSizeCallback(handle_, resizeCallback);
        glfwSetWindowCloseCallback(handle_, closeCallback);
        glfwSetWindowFocusCallback(handle_, focusCallback);
        glfwSetWindowIconifyCallback(handle_, iconifyCallback);
        glfwSetKeyCallback(handle_, keyCallback);
        glfwSetMouseButtonCallback(handle_, mouseButtonCallback);
        glfwSetCursorPosCallback(handle_, mousePositionCallback);
        glfwSetScrollCallback(handle_, mouseScrollCallback);
    }

    Window::~Window() {
        if (handle_) {
            glfwDestroyWindow(handle_);

            handle_ = nullptr;
        }
    }

    void Window::setExtent(const glm::uvec2& extent) {
        if (!handle_) {
            throw std::runtime_error("Illegal call to app::Window::setExtent(): Window is invalid");
        }

        extent_ = extent;

        glfwSetWindowSize(handle_, static_cast<std::int32_t>(extent_.x), static_cast<std::int32_t>(extent_.y));
    }

    void Window::setTitle(const std::string& title) {
        if (!handle_) {
            throw std::runtime_error("Illegal call to app::Window::setTitle(): Window is invalid");
        }

        title_ = title;

        glfwSetWindowTitle(handle_, title_.c_str());
    }

    void Window::setVisibility(const WindowVisibility& visibility) {
        if (!handle_) {
            throw std::runtime_error("Illegal call to app::Window::setVisibility(): Window is invalid");
        }

        if (visibility == visibility_) {
            return;
        }

        switch (visibility) {
            case WindowVisibility::WINDOWED:
                if (visibility_ == WindowVisibility::MINIMISED) {
                    glfwRestoreWindow(handle_);
                }
                else if (visibility_ == WindowVisibility::FULLSCREEN) {
                    std::uint32_t width = lastWindowedExtent_.x;
                    std::uint32_t height = lastWindowedExtent_.y;

                    glfwSetWindowMonitor(handle_, nullptr, GLFW_DONT_CARE, GLFW_DONT_CARE, static_cast<std::int32_t>(width), static_cast<std::int32_t>(height), 0);
                }

                break;

            case WindowVisibility::MINIMISED:
                glfwIconifyWindow(handle_);
                break;

            case WindowVisibility::FULLSCREEN: {
                GLFWmonitor* monitor = glfwGetPrimaryMonitor();
                const GLFWvidmode* mode = glfwGetVideoMode(monitor);

                lastWindowedExtent_ = extent_;

                glfwSetWindowMonitor(
                    handle_,
                    monitor,
                    0, 0,
                    mode->width, mode->height,
                    mode->refreshRate);
                break;
            }
        }

        visibility_ = visibility;
    }

    const glm::uvec2& Window::extent() const {
        if (!handle_) {
            throw std::runtime_error("Illegal call to app::Window::extent(): Window is invalid");
        }

        return extent_;
    }

    const std::string& Window::getTitle() const {
        if (!handle_) {
            throw std::runtime_error("Illegal call to app::Window::getTitle(): Window is invalid");
        }

        return title_;
    }

    const WindowVisibility& Window::getVisibility() const {
        if (!handle_) {
            throw std::runtime_error("Illegal call to app::Window::getVisibility(): Window is invalid");
        }

        return visibility_;
    }

    bool Window::hasEvents() const {
        return !events_.empty();
    }

    WindowEvent Window::getNextEvent() {
        if (events_.empty()) {
            throw std::runtime_error("Illegal call to app::Window::getNextEvent(): No events to process");
        }

        WindowEvent event = events_.front();

        events_.pop();

        return event;
    }

    GLFWwindow*& Window::getAgnosticHandle() {
        if (!handle_) {
            throw std::runtime_error("Illegal call to app::Window::getAgnosticHandle(): Window is invalid");
        }

        return handle_;
    }

    void Window::resizeCallback(GLFWwindow* window, int width, int height) {
        auto self = static_cast<Window*>(glfwGetWindowUserPointer(window));

        if (!self) {
            return;
        }

        self->extent_ = {
            static_cast<std::uint32_t>(width),
            static_cast<std::uint32_t>(height),
        };

        WindowResizeEventInfo eventInfo = {
            .extent = self->extent_,
        };

        WindowEvent event = {
            .type = WindowEventType::RESIZED,
            .info = {
                .resize = eventInfo,
            },
        };

        self->events_.push(event);
    }

    void Window::closeCallback(GLFWwindow* window) {
        auto self = static_cast<Window*>(glfwGetWindowUserPointer(window));

        if (!self) {
            return;
        }

        WindowEvent event = {
            .type = WindowEventType::CLOSED,
            .info = {},
        };

        self->events_.push(event);
    }

    void Window::focusCallback(GLFWwindow* window, int focused) {
        auto self = static_cast<Window*>(glfwGetWindowUserPointer(window));

        if (!self) {
            return;
        }

        WindowEvent event = {
            .type = focused ? WindowEventType::FOCUSED : WindowEventType::UNFOCUSED,
            .info = {},
        };

        self->events_.push(event);
    }

    void Window::iconifyCallback(GLFWwindow* window, int iconified) {
        auto self = static_cast<Window*>(glfwGetWindowUserPointer(window));

        if (!self) {
            return;
        }

        WindowEvent event = {
            .type = iconified ? WindowEventType::MINIMISED : WindowEventType::RESTORED,
            .info = {},
        };

        if (event.type == WindowEventType::MINIMISED) {
            self->lastVisibility_ = self->visibility_;
            self->visibility_ = WindowVisibility::MINIMISED;
        }
        else {
            self->visibility_ = self->lastVisibility_;
        }

        self->events_.push(event);
    }

    void Window::keyCallback(GLFWwindow* window, int key, int, int action, int) {
        auto self = static_cast<Window*>(glfwGetWindowUserPointer(window));

        if (!self) {
            return;
        }

        Key mappedKey = mapKey(key);

        WindowEvent event = {};

        if (action == GLFW_PRESS) {
            WindowKeyPressedEventInfo keyPressedEventInfo = {
                .key = mappedKey,
            };

            event = {
                .type = WindowEventType::KEY_PRESSED,
                .info = {
                    .keyPress = keyPressedEventInfo,
                },
            };
        }
        else if (action == GLFW_RELEASE) {
            WindowKeyReleasedEventInfo keyReleasedEventInfo = {
                .key = mappedKey,
            };

            event = {
                .type = WindowEventType::KEY_RELEASED,
                .info = {
                    .keyRelease = keyReleasedEventInfo,
                },
            };
        }

        self->events_.push(event);
    }

    void Window::mouseButtonCallback(GLFWwindow* window, int button, int action, int) {
        auto self = static_cast<Window*>(glfwGetWindowUserPointer(window));

        if (!self) {
            return;
        }

        MouseButton mappedButton = mapMouseButton(button);

        WindowEvent event = {};

        if (action == GLFW_PRESS) {
            WindowMouseButtonPressedEventInfo mouseButtonPressedEventInfo = {
                .button = mappedButton,
            };

            event = {
                .type = WindowEventType::MOUSE_BUTTON_PRESSED,
                .info = {
                    .mouseButtonPress = mouseButtonPressedEventInfo,
                },
            };
        }
        else if (action == GLFW_RELEASE) {
            WindowMouseButtonReleasedEventInfo mouseButtonReleasedEventInfo = {
                .button = mappedButton,
            };

            event = {
                .type = WindowEventType::MOUSE_BUTTON_RELEASED,
                .info = {
                    .mouseButtonRelease = mouseButtonReleasedEventInfo,
                },
            };
        }

        self->events_.push(event);
    }

    void Window::mousePositionCallback(GLFWwindow* window, double x, double y) {
        auto self = static_cast<Window*>(glfwGetWindowUserPointer(window));

        if (!self) {
            return;
        }

        WindowMouseMovedEventInfo mouseMoveEventInfo = {
            .position = {x, y},
        };

        WindowEvent event = {
            .type = WindowEventType::MOUSE_MOVED,
            .info = {
                .mouseMove = mouseMoveEventInfo,
            },
        };

        self->events_.push(event);
    }

    void Window::mouseScrollCallback(GLFWwindow* window, double x, double y) {
        auto self = static_cast<Window*>(glfwGetWindowUserPointer(window));

        if (!self) {
            return;
        }

        WindowMouseScrolledEventInfo mouseScrollEventInfo = {
            .offset = {x, y},
        };

        WindowEvent event = {
            .type = WindowEventType::MOUSE_SCROLLED,
            .info = {
                .mouseScroll = mouseScrollEventInfo,
            },
        };

        self->events_.push(event);
    }

    Key Window::mapKey(int key) {
        switch (key) {
            case GLFW_KEY_A:
                return Key::A;

            case GLFW_KEY_B:
                return Key::B;

            case GLFW_KEY_C:
                return Key::C;

            case GLFW_KEY_D:
                return Key::D;

            case GLFW_KEY_E:
                return Key::E;

            case GLFW_KEY_F:
                return Key::F;

            case GLFW_KEY_G:
                return Key::G;

            case GLFW_KEY_H:
                return Key::H;

            case GLFW_KEY_I:
                return Key::I;

            case GLFW_KEY_J:
                return Key::J;

            case GLFW_KEY_K:
                return Key::K;

            case GLFW_KEY_L:
                return Key::L;

            case GLFW_KEY_M:
                return Key::M;

            case GLFW_KEY_N:
                return Key::N;

            case GLFW_KEY_O:
                return Key::O;

            case GLFW_KEY_P:
                return Key::P;

            case GLFW_KEY_Q:
                return Key::Q;

            case GLFW_KEY_R:
                return Key::R;

            case GLFW_KEY_S:
                return Key::S;

            case GLFW_KEY_T:
                return Key::T;

            case GLFW_KEY_U:
                return Key::U;

            case GLFW_KEY_V:
                return Key::V;

            case GLFW_KEY_W:
                return Key::W;

            case GLFW_KEY_X:
                return Key::X;

            case GLFW_KEY_Y:
                return Key::Y;

            case GLFW_KEY_Z:
                return Key::Z;

            case GLFW_KEY_F1:
                return Key::F1;

            case GLFW_KEY_F2:
                return Key::F2;

            case GLFW_KEY_F3:
                return Key::F3;

            case GLFW_KEY_F4:
                return Key::F4;

            case GLFW_KEY_F5:
                return Key::F5;

            case GLFW_KEY_F6:
                return Key::F6;

            case GLFW_KEY_F7:
                return Key::F7;

            case GLFW_KEY_F8:
                return Key::F8;

            case GLFW_KEY_F9:
                return Key::F9;

            case GLFW_KEY_F10:
                return Key::F10;

            case GLFW_KEY_F11:
                return Key::F11;

            case GLFW_KEY_F12:
                return Key::F12;

            case GLFW_KEY_SPACE:
                return Key::SPACE;

            case GLFW_KEY_LEFT_CONTROL:
                return Key::LCTRL;

            case GLFW_KEY_RIGHT_CONTROL:
                return Key::RCTRL;

            case GLFW_KEY_LEFT_SUPER:
                return Key::LSUPER;

            case GLFW_KEY_RIGHT_SUPER:
                return Key::RSUPER;

            case GLFW_KEY_LEFT_ALT:
                return Key::LALT;

            case GLFW_KEY_RIGHT_ALT:
                return Key::RALT;

            case GLFW_KEY_CAPS_LOCK:
                return Key::CAPSLOCK;

            case GLFW_KEY_TAB:
                return Key::TAB;

            case GLFW_KEY_ESCAPE:
                return Key::ESCAPE;

            case GLFW_KEY_LEFT_SHIFT:
                return Key::LSHIFT;

            case GLFW_KEY_RIGHT_SHIFT:
                return Key::RSHIFT;

            case GLFW_KEY_HOME:
                return Key::HOME;

            case GLFW_KEY_END:
                return Key::END;

            case GLFW_KEY_PAGE_UP:
                return Key::PGUP;

            case GLFW_KEY_PAGE_DOWN:
                return Key::PGDN;

            case GLFW_KEY_INSERT:
                return Key::INSERT;

            case GLFW_KEY_DELETE:
                return Key::DELETE;

            case GLFW_KEY_ENTER:
                return Key::ENTER;

            case GLFW_KEY_BACKSPACE:
                return Key::BACKSPACE;

            case GLFW_KEY_GRAVE_ACCENT:
                return Key::GRAVE_ACCENT;

            case GLFW_KEY_EQUAL:
                return Key::EQUAL;

            case GLFW_KEY_MINUS:
                return Key::MINUS;

            case GLFW_KEY_SLASH:
                return Key::SLASH;

            case GLFW_KEY_BACKSLASH:
                return Key::BACKSLASH;

            case GLFW_KEY_SEMICOLON:
                return Key::SEMICOLON;

            case GLFW_KEY_APOSTROPHE:
                return Key::APOSTROPHE;

            case GLFW_KEY_COMMA:
                return Key::COMMA;

            case GLFW_KEY_PERIOD:
                return Key::PERIOD;

            case GLFW_KEY_0:
                return Key::ROW_0;

            case GLFW_KEY_1:
                return Key::ROW_1;

            case GLFW_KEY_2:
                return Key::ROW_2;

            case GLFW_KEY_3:
                return Key::ROW_3;

            case GLFW_KEY_4:
                return Key::ROW_4;

            case GLFW_KEY_5:
                return Key::ROW_5;

            case GLFW_KEY_6:
                return Key::ROW_6;

            case GLFW_KEY_7:
                return Key::ROW_7;

            case GLFW_KEY_8:
                return Key::ROW_8;

            case GLFW_KEY_9:
                return Key::ROW_9;

            case GLFW_KEY_KP_ADD:
                return Key::KEYPAD_PLUS;

            case GLFW_KEY_KP_SUBTRACT:
                return Key::KEYPAD_MINUS;

            case GLFW_KEY_KP_0:
                return Key::KEYPAD_0;

            case GLFW_KEY_KP_1:
                return Key::KEYPAD_1;

            case GLFW_KEY_KP_2:
                return Key::KEYPAD_2;

            case GLFW_KEY_KP_3:
                return Key::KEYPAD_3;

            case GLFW_KEY_KP_4:
                return Key::KEYPAD_4;

            case GLFW_KEY_KP_5:
                return Key::KEYPAD_5;

            case GLFW_KEY_KP_6:
                return Key::KEYPAD_6;

            case GLFW_KEY_KP_7:
                return Key::KEYPAD_7;

            case GLFW_KEY_KP_8:
                return Key::KEYPAD_8;

            case GLFW_KEY_KP_9:
                return Key::KEYPAD_9;

            case GLFW_KEY_UP:
                return Key::UP;

            case GLFW_KEY_DOWN:
                return Key::DOWN;

            case GLFW_KEY_LEFT:
                return Key::LEFT;

            case GLFW_KEY_RIGHT:
                return Key::RIGHT;
        }

        throw;
    }

    MouseButton Window::mapMouseButton(int button) {
        switch (button) {
            case GLFW_MOUSE_BUTTON_LEFT:
                return MouseButton::LEFT;

            case GLFW_MOUSE_BUTTON_RIGHT:
                return MouseButton::RIGHT;

            case GLFW_MOUSE_BUTTON_MIDDLE:
                return MouseButton::MIDDLE;
        }

        throw;
    }
}