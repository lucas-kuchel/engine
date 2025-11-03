#pragma once

#include <app/window.hpp>

#include <array>

#include <entt/entt.hpp>
#include <magic_enum/magic_enum.hpp>

namespace engine {
    struct KeyPressEvent {
        app::Key key;
    };

    struct KeyHoldEvent {
        app::Key key;
    };

    struct KeyReleaseEvent {
        app::Key key;
    };

    struct ButtonPressEvent {
        app::MouseButton button;
    };

    struct ButtonHoldEvent {
        app::MouseButton button;
    };

    struct ButtonReleaseEvent {
        app::MouseButton button;
    };

    struct MouseMoveEvent {
        glm::vec2 lastPosition;
        glm::vec2 position;
    };

    struct MouseScrollEvent {
        glm::vec2 lastOffset;
        glm::vec2 offset;
    };

    class InputManager {
    public:
        void update();

        void emitToDispatcherDeferred(entt::dispatcher& dispatcher);
        void emitToDispatcherImmediate(entt::dispatcher& dispatcher);

        void updateKeymaps(const app::WindowKeyPressedEventInfo& keyPressEvent);
        void updateKeymaps(const app::WindowKeyReleasedEventInfo& keyReleaseEvent);
        void updateButtonMaps(const app::WindowMouseButtonPressedEventInfo& buttonPressEvent);
        void updateButtonMaps(const app::WindowMouseButtonReleasedEventInfo& buttonReleaseEvent);
        void updateMousePosition(const app::WindowMouseMovedEventInfo& mouseMovedEvent);
        void updateMouseScroll(const app::WindowMouseScrolledEventInfo& mouseScrolledEvent);

        bool pressed(app::Key key) const;
        bool held(app::Key key) const;
        bool released(app::Key key) const;

        bool pressed(app::MouseButton button) const;
        bool held(app::MouseButton button) const;
        bool released(app::MouseButton button) const;

        glm::vec2 mousePosition() const;
        glm::vec2 mouseScroll() const;

        glm::vec2 mousePositionDelta() const;
        glm::vec2 mouseScrollDelta() const;

        glm::vec2 lastMousePosition() const;
        glm::vec2 lastMouseScroll() const;

        std::span<bool> getKeysPressed();
        std::span<bool> getKeysHeld();
        std::span<bool> getKeysReleased();

        std::span<const bool> getKeysPressed() const;
        std::span<const bool> getKeysHeld() const;
        std::span<const bool> getKeysReleased() const;

        std::span<bool> getButtonsPressed();
        std::span<bool> getButtonsHeld();
        std::span<bool> getButtonsReleased();

        std::span<const bool> getButtonsPressed() const;
        std::span<const bool> getButtonsHeld() const;
        std::span<const bool> getButtonsReleased() const;

        static constexpr std::size_t getKeyCount();
        static constexpr std::size_t getMouseButtonCount();

    private:
        std::array<bool, magic_enum::enum_count<app::Key>()> keysPressed_ = {};
        std::array<bool, magic_enum::enum_count<app::Key>()> keysHeld_ = {};
        std::array<bool, magic_enum::enum_count<app::Key>()> keysReleased_ = {};

        std::array<bool, magic_enum::enum_count<app::MouseButton>()> buttonsPressed_ = {};
        std::array<bool, magic_enum::enum_count<app::MouseButton>()> buttonsHeld_ = {};
        std::array<bool, magic_enum::enum_count<app::MouseButton>()> buttonsReleased_ = {};

        glm::vec2 mousePosition_ = {0.0f, 0.0f};
        glm::vec2 mouseScroll_ = {0.0f, 0.0f};

        glm::vec2 mousePositionDelta_ = {0.0f, 0.0f};
        glm::vec2 mouseScrollDelta_ = {0.0f, 0.0f};

        glm::vec2 lastMousePosition_ = {0.0f, 0.0f};
        glm::vec2 lastMouseScroll_ = {0.0f, 0.0f};
    };
}