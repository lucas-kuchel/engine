#pragma once

#include <components/tile.hpp>
#include <components/world.hpp>

#include <entt/entt.hpp>

namespace engine {
    class Engine;
    class TilePool;
}

namespace systems {
    void integrateMovements(engine::Engine& engine);
    void transformInstances(engine::Engine& engine, engine::TilePool& tilePool);

    template <typename T>
    void cacheLasts(entt::registry& registry) {
        for (auto& entity : registry.view<components::Last<T>, T>()) {
            auto& current = registry.get<T>(entity);
            auto& last = registry.get<components::Last<T>>(entity);

            last.value = current;
        }
    }
}