#pragma once

#include <entt/entt.hpp>
#include <glm/glm.hpp>

namespace engine::systems {
    void cameraFollowCharacter(entt::registry& registry, entt::entity& character, entt::entity& camera, glm::vec2 mousePosition, float deltaTime);
}