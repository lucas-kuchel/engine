#pragma once

#include <vulkanite/window/configuration.hpp>
#include <vulkanite/window/window.hpp>

#include <array>

#include <entt/entt.hpp>
#include <magic_enum/magic_enum.hpp>

namespace engine {
    struct KeyPressEvent {
        vulkanite::window::Key key;
    };

    struct KeyHoldEvent {
        vulkanite::window::Key key;
    };

    struct KeyReleaseEvent {
        vulkanite::window::Key key;
    };

    struct ButtonPressEvent {
        vulkanite::window::MouseButton button;
    };

    struct ButtonHoldEvent {
        vulkanite::window::MouseButton button;
    };

    struct ButtonReleaseEvent {
        vulkanite::window::MouseButton button;
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

        void updateKeymaps(const vulkanite::window::KeyPressedEventInfo& keyPressEvent);
        void updateKeymaps(const vulkanite::window::KeyReleasedEventInfo& keyReleaseEvent);
        void updateButtonMaps(const vulkanite::window::MouseButtonPressedEventInfo& buttonPressEvent);
        void updateButtonMaps(const vulkanite::window::MouseButtonReleasedEventInfo& buttonReleaseEvent);
        void updateMousePosition(const vulkanite::window::MouseMovedEventInfo& mouseMovedEvent);
        void updateMouseScroll(const vulkanite::window::MouseScrolledEventInfo& mouseScrolledEvent);

        bool pressed(vulkanite::window::Key key) const;
        bool held(vulkanite::window::Key key) const;
        bool released(vulkanite::window::Key key) const;

        bool pressed(vulkanite::window::MouseButton button) const;
        bool held(vulkanite::window::MouseButton button) const;
        bool released(vulkanite::window::MouseButton button) const;

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
        std::array<bool, magic_enum::enum_count<vulkanite::window::Key>()> keysPressed_ = {};
        std::array<bool, magic_enum::enum_count<vulkanite::window::Key>()> keysHeld_ = {};
        std::array<bool, magic_enum::enum_count<vulkanite::window::Key>()> keysReleased_ = {};

        std::array<bool, magic_enum::enum_count<vulkanite::window::MouseButton>()> buttonsPressed_ = {};
        std::array<bool, magic_enum::enum_count<vulkanite::window::MouseButton>()> buttonsHeld_ = {};
        std::array<bool, magic_enum::enum_count<vulkanite::window::MouseButton>()> buttonsReleased_ = {};

        glm::vec2 mousePosition_ = {0.0f, 0.0f};
        glm::vec2 mouseScroll_ = {0.0f, 0.0f};

        glm::vec2 mousePositionDelta_ = {0.0f, 0.0f};
        glm::vec2 mouseScrollDelta_ = {0.0f, 0.0f};

        glm::vec2 lastMousePosition_ = {0.0f, 0.0f};
        glm::vec2 lastMouseScroll_ = {0.0f, 0.0f};
    };
}