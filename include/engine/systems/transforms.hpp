#pragma once

#include <engine/components/tile.hpp>

#include <entt/entt.hpp>

namespace engine::systems {
    void cachePositions(entt::registry& registry);
    void integrateMovements(entt::registry& registry, float deltaTime);
    void transformInstances(entt::registry& registry, std::span<components::TileInstance> instances);
}