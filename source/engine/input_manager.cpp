#include <app/configuration.hpp>
#include <engine/input_manager.hpp>

void engine::InputManager::update() {
    lastMousePosition_ = mousePosition_;
    lastMouseScroll_ = mouseScroll_;

    for (std::size_t i = 0; i < getKeyCount(); i++) {
        keysPressed_[i] = false;
        keysReleased_[i] = false;
    }

    for (std::size_t i = 0; i < getMouseButtonCount(); i++) {
        buttonsPressed_[i] = false;
        buttonsReleased_[i] = false;
    }
}

void engine::InputManager::emitToDispatcherDeferred(entt::dispatcher& dispatcher) {
    constexpr auto keys = magic_enum::enum_values<app::Key>();
    constexpr auto buttons = magic_enum::enum_values<app::MouseButton>();

    for (std::size_t i = 0; i < getKeyCount(); i++) {
        if (keysPressed_[i]) {
            dispatcher.enqueue(KeyPressEvent{keys[i]});
        }

        if (keysHeld_[i]) {
            dispatcher.enqueue(KeyHoldEvent{keys[i]});
        }

        if (keysReleased_[i]) {
            dispatcher.enqueue(KeyReleaseEvent{keys[i]});
        }
    }

    for (std::size_t i = 0; i < getMouseButtonCount(); i++) {
        if (buttonsPressed_[i]) {
            dispatcher.enqueue(ButtonPressEvent{buttons[i]});
        }

        if (buttonsHeld_[i]) {
            dispatcher.enqueue(ButtonHoldEvent{buttons[i]});
        }

        if (buttonsReleased_[i]) {
            dispatcher.enqueue(ButtonReleaseEvent{buttons[i]});
        }
    }

    if (lastMousePosition_ != mousePosition_) {
        dispatcher.enqueue(MouseMoveEvent{lastMousePosition_, mousePosition_});
    }

    if (lastMouseScroll_ != lastMouseScroll_) {
        dispatcher.enqueue(MouseMoveEvent{lastMouseScroll_, mouseScroll_});
    }
}

void engine::InputManager::emitToDispatcherImmediate(entt::dispatcher& dispatcher) {
    constexpr auto keys = magic_enum::enum_values<app::Key>();
    constexpr auto buttons = magic_enum::enum_values<app::MouseButton>();

    for (std::size_t i = 0; i < getKeyCount(); i++) {
        if (keysPressed_[i]) {
            dispatcher.trigger(KeyPressEvent{keys[i]});
        }

        if (keysHeld_[i]) {
            dispatcher.trigger(KeyHoldEvent{keys[i]});
        }

        if (keysReleased_[i]) {
            dispatcher.trigger(KeyReleaseEvent{keys[i]});
        }
    }

    for (std::size_t i = 0; i < getMouseButtonCount(); i++) {
        if (buttonsPressed_[i]) {
            dispatcher.trigger(ButtonPressEvent{buttons[i]});
        }

        if (buttonsHeld_[i]) {
            dispatcher.trigger(ButtonHoldEvent{buttons[i]});
        }

        if (buttonsReleased_[i]) {
            dispatcher.trigger(ButtonReleaseEvent{buttons[i]});
        }
    }

    if (lastMousePosition_ != mousePosition_) {
        dispatcher.trigger(MouseMoveEvent{lastMousePosition_, mousePosition_});
    }

    if (lastMouseScroll_ != lastMouseScroll_) {
        dispatcher.trigger(MouseMoveEvent{lastMouseScroll_, mouseScroll_});
    }
}

void engine::InputManager::updateKeymaps(const app::WindowKeyPressedEventInfo& keyPressEvent) {
    std::size_t index = static_cast<std::size_t>(keyPressEvent.key);

    keysPressed_[index] = true;
    keysHeld_[index] = true;
}

void engine::InputManager::updateKeymaps(const app::WindowKeyReleasedEventInfo& keyReleaseEvent) {
    std::size_t index = static_cast<std::size_t>(keyReleaseEvent.key);

    keysHeld_[index] = false;
    keysReleased_[index] = true;
}

void engine::InputManager::updateButtonMaps(const app::WindowMouseButtonPressedEventInfo& buttonPressEvent) {
    std::size_t index = static_cast<std::size_t>(buttonPressEvent.button);

    buttonsPressed_[index] = true;
    buttonsHeld_[index] = true;
}

void engine::InputManager::updateButtonMaps(const app::WindowMouseButtonReleasedEventInfo& buttonReleaseEvent) {
    std::size_t index = static_cast<std::size_t>(buttonReleaseEvent.button);

    buttonsHeld_[index] = false;
    buttonsReleased_[index] = true;
}

void engine::InputManager::updateMousePosition(const app::WindowMouseMovedEventInfo& mouseMovedEvent) {
    mousePosition_ = mouseMovedEvent.position;
}

void engine::InputManager::updateMouseScroll(const app::WindowMouseScrolledEventInfo& mouseScrolledEvent) {
    mouseScroll_ += mouseScrolledEvent.offset;
}

bool engine::InputManager::pressed(app::Key key) const {
    return keysPressed_[static_cast<std::size_t>(key)];
}

bool engine::InputManager::held(app::Key key) const {
    return keysHeld_[static_cast<std::size_t>(key)];
}

bool engine::InputManager::released(app::Key key) const {
    return keysReleased_[static_cast<std::size_t>(key)];
}

bool engine::InputManager::pressed(app::MouseButton button) const {
    return buttonsPressed_[static_cast<std::size_t>(button)];
}

bool engine::InputManager::held(app::MouseButton button) const {
    return buttonsHeld_[static_cast<std::size_t>(button)];
}

bool engine::InputManager::released(app::MouseButton button) const {
    return buttonsReleased_[static_cast<std::size_t>(button)];
}

glm::vec2 engine::InputManager::mousePosition() const {
    return mousePosition_;
}

glm::vec2 engine::InputManager::mouseScroll() const {
    return mouseScroll_;
}

glm::vec2 engine::InputManager::mousePositionDelta() const {
    return mousePosition_ - lastMousePosition_;
}

glm::vec2 engine::InputManager::mouseScrollDelta() const {
    return mouseScroll_ - lastMouseScroll_;
}

glm::vec2 engine::InputManager::lastMousePosition() const {
    return lastMousePosition_;
}

glm::vec2 engine::InputManager::lastMouseScroll() const {
    return lastMouseScroll_;
}

std::span<bool> engine::InputManager::getKeysPressed() {
    return keysPressed_;
}

std::span<bool> engine::InputManager::getKeysHeld() {
    return keysHeld_;
}

std::span<bool> engine::InputManager::getKeysReleased() {
    return keysReleased_;
}

std::span<const bool> engine::InputManager::getKeysPressed() const {
    return keysPressed_;
}

std::span<const bool> engine::InputManager::getKeysHeld() const {
    return keysHeld_;
}

std::span<const bool> engine::InputManager::getKeysReleased() const {
    return keysReleased_;
}

std::span<bool> engine::InputManager::getButtonsPressed() {
    return buttonsPressed_;
}

std::span<bool> engine::InputManager::getButtonsHeld() {
    return buttonsHeld_;
}

std::span<bool> engine::InputManager::getButtonsReleased() {
    return buttonsReleased_;
}

std::span<const bool> engine::InputManager::getButtonsPressed() const {
    return buttonsPressed_;
}

std::span<const bool> engine::InputManager::getButtonsHeld() const {
    return buttonsHeld_;
}

std::span<const bool> engine::InputManager::getButtonsReleased() const {
    return buttonsReleased_;
}

constexpr std::size_t engine::InputManager::getKeyCount() {
    return magic_enum::enum_count<app::Key>();
}

constexpr std::size_t engine::InputManager::getMouseButtonCount() {
    return magic_enum::enum_count<app::MouseButton>();
}