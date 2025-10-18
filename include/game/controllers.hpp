#pragma once

#include <app/window.hpp>

#include <entt/entt.hpp>

namespace game {
    struct PositionController {
        app::Key forwardBinding;
        app::Key backwardBinding;
        app::Key leftBinding;
        app::Key rightBinding;
    };

    void updatePositionControllers(entt::registry& registry, std::span<bool> keymap);
}