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
        acceleration.acceleration = {0.0f, 0.0f, 0.0f};
        position.position += velocity.velocity * deltaTime;
    }
}

void engine::systems::transformInstances(entt::registry& registry, std::span<components::Tile> instances) {
    for (auto& entity : registry.view<components::Proxy<components::Tile>>(entt::exclude<components::StaticTileTag>)) {
        auto& tileInstance = instances[registry.get<components::Proxy<components::Tile>>(entity).index];

        glm::vec3 position = {0.0f, 0.0f, 0.0f};
        glm::vec2 scale = {1.0f, 1.0f};
        float rotation = 0.0f;

        if (registry.all_of<components::Position>(entity)) {
            position = registry.get<components::Position>(entity).position;
        }

        if (registry.all_of<components::Rotation>(entity)) {
            rotation = registry.get<components::Rotation>(entity).angle;
        }

        if (registry.all_of<components::Scale>(entity)) {
            scale = registry.get<components::Scale>(entity).scale;
        }

        float sin = std::sin(glm::radians(rotation));
        float cos = std::cos(glm::radians(rotation));

        tileInstance.transform.position = glm::vec4(position, 1.0f);
        tileInstance.transform.model = glm::mat2{
            glm::vec2{cos * scale.x, -sin * scale.y},
            glm::vec2{sin * scale.x, cos * scale.y},
        };
    }
}
