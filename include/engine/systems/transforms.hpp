#pragma once

#include <entt/entt.hpp>

namespace engine::systems {
    void integrateMovements(entt::registry& registry, float deltaTime);
}