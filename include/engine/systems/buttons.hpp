#pragma once

#include <entt/entt.hpp>
#include <glm/glm.hpp>

namespace engine::systems {
    void createButtons(entt::registry& registry);
    void testButtons(entt::registry& registry, glm::vec2 mousePosition, glm::vec2 lastMousePosition);
    void animateButtons(entt::registry& registry, float deltaTime);
}