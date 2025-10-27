#pragma once

#include <components/camera.hpp>
#include <engine/engine.hpp>
#include <entt/entt.hpp>
#include <glm/glm.hpp>

namespace systems {
    void createButtons(entt::registry& registry, engine::Engine& engine);
    void testButtons(entt::registry& registry, glm::vec2 mousePosition, glm::vec2 lastMousePosition, components::Bounds& bounds);
    void animateButtons(entt::registry& registry, float deltaTime);
}