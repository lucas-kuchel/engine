#include <engine/components/camera.hpp>
#include <engine/components/entity_tags.hpp>
#include <engine/components/proxy.hpp>
#include <engine/components/tile.hpp>
#include <engine/components/transforms.hpp>
#include <engine/systems/transforms.hpp>

void engine::systems::cachePositions(entt::registry& registry) {
    for (auto& entity : registry.view<components::Position>()) {
        auto& position = registry.get<components::Position>(entity);
        position.lastPosition = position.position;
    }
}

void engine::systems::integrateMovements(entt::registry& registry, float deltaTime) {
    for (auto& entity : registry.view<components::Velocity, components::Acceleration, components::Position>()) {
        auto& velocity = registry.get<components::Velocity>(entity);
        auto& acceleration = registry.get<components::Acceleration>(entity);
        auto& position = registry.get<components::Position>(entity);

        velocity.velocity += acceleration.acceleration * deltaTime;
        acceleration.acceleration = {0.0f, 0.0f};
        position.position += velocity.velocity * deltaTime;
    }
}

void engine::systems::transformInstances(entt::registry& registry, std::span<components::TileInstance> instances) {
    for (auto& entity : registry.view<components::Proxy<components::TileInstance>, components::Position, components::Scale>(entt::exclude<components::TileTag>)) {
        auto& proxy = registry.get<components::Proxy<components::TileInstance>>(entity);
        auto& position = registry.get<components::Position>(entity);
        auto& scale = registry.get<components::Scale>(entity);
        auto& tileInstance = instances[proxy.index];

        tileInstance.position = glm::vec3{position.position, position.depth};
        tileInstance.scale = scale.scale;
    }
}
