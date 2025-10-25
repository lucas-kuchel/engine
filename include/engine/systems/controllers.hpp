#pragma once

#include <app/window.hpp>

#include <entt/entt.hpp>

namespace engine::systems {
    void updatePositionControllers(entt::registry& registry, std::span<bool> keymap);
    void clampSpeeds(entt::registry& registry);
}