#pragma once

#include <entt/entt.hpp>
#include <glm/glm.hpp>

namespace engine::systems {
    void updateButtons(entt::registry& registry, glm::vec2 mousePosition, float deltaTime);
}