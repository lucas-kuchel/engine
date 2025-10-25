#pragma once

#include <engine/components/tile.hpp>
#include <engine/components/world.hpp>

#include <entt/entt.hpp>

namespace engine::systems {
    void integrateMovements(entt::registry& registry, float deltaTime);
    void integrateFriction(entt::registry& registry, components::World& world, float deltaTime);
    void transformInstances(entt::registry& registry, std::span<components::TileInstance> instances);

    template <typename T>
    void cacheLasts(entt::registry& registry) {
        for (auto& entity : registry.view<components::Last<T>, T>()) {
            auto& current = registry.get<T>(entity);
            auto& last = registry.get<components::Last<T>>(entity);

            last.value = current;
        }
    }
}