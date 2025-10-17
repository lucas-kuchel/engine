#include <game/transforms.hpp>

namespace game {
    void integrate(entt::registry& registry, float deltaTime) {
        auto view = registry.view<Velocity, Acceleration, Position>();

        for (auto& entity : view) {
            auto& velocity = registry.get<Velocity>(entity);
            auto& acceleration = registry.get<Acceleration>(entity);
            auto& position = registry.get<Position>(entity);

            velocity.velocity += acceleration.acceleration * deltaTime;
            position.position += velocity.velocity * deltaTime;
        }
    }

    void updateScales(entt::registry& registry) {
        auto view = registry.view<Scale, Transform>();

        for (auto& entity : view) {
            auto& transform = registry.get<Transform>(entity);
            auto& scale = registry.get<Scale>(entity);

            transform.matrix = glm::scale(transform.matrix, scale.scale);
        }
    }

    void updateShears(entt::registry& registry) {
        auto view = registry.view<Shear, Transform>();

        for (auto& entity : view) {
            auto& transform = registry.get<Transform>(entity);
            auto& shear = registry.get<Shear>(entity);

            glm::vec2 yz = {shear.shear.y, shear.shear.z};
            glm::vec2 xz = {shear.shear.x, shear.shear.z};
            glm::vec2 xy = {shear.shear.x, shear.shear.y};

            transform.matrix = glm::shear(transform.matrix, shear.reference, yz, xz, xy);
        }
    }

    void updateRotations(entt::registry& registry) {
        auto view = registry.view<Rotation, Transform>();

        for (auto& entity : view) {
            auto& transform = registry.get<Transform>(entity);
            auto& rotation = registry.get<Rotation>(entity);

            transform.matrix = glm::rotate(transform.matrix, glm::radians(rotation.rotation.x), glm::vec3{1.0f, 0.0f, 0.0f});
            transform.matrix = glm::rotate(transform.matrix, glm::radians(rotation.rotation.y), glm::vec3{0.0f, 1.0f, 0.0f});
            transform.matrix = glm::rotate(transform.matrix, glm::radians(rotation.rotation.z), glm::vec3{0.0f, 0.0f, 1.0f});
        }
    }

    void updatePositions(entt::registry& registry) {
        auto view = registry.view<Position, Transform>();

        for (auto& entity : view) {
            auto& transform = registry.get<Transform>(entity);
            auto& position = registry.get<Position>(entity);

            transform.matrix = glm::translate(transform.matrix, position.position);
        }
    }
}