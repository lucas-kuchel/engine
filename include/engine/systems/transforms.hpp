#pragma once

#include <entt/entt.hpp>

namespace engine::systems {
    void cachePositions(entt::registry& registry);
    void integrateMovements(entt::registry& registry, float deltaTime);
}