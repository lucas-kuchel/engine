#pragma once

#include <entt/entt.hpp>

namespace engine::systems {
    void cameraFollowCharacter(entt::registry& registry, entt::entity& character, entt::entity& camera, float deltaTime);
}