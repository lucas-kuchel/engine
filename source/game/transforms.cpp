#include <game/mesh.hpp>
#include <game/transforms.hpp>

#include <glm/gtc/matrix_transform.hpp>

namespace game {
    void integrate(entt::registry& registry, float deltaTime) {
        for (auto& entity : registry.view<Velocity, Acceleration, Position>()) {
            auto& velocity = registry.get<Velocity>(entity);
            auto& acceleration = registry.get<Acceleration>(entity);
            auto& position = registry.get<Position>(entity);

            velocity.velocity += acceleration.acceleration * deltaTime;
            velocity.velocity *= std::pow(0.95f, deltaTime);
            position.position += velocity.velocity * deltaTime;

            acceleration.acceleration = {0.0f, 0.0f, 0.0f};
        }
    }

    void transform(entt::registry& registry) {
        for (auto entity : registry.view<MeshTransform>()) {
            auto& transform = registry.get<MeshTransform>(entity);

            glm::vec3 position = {0.0f, 0.0f, 0.0f};
            glm::vec2 scale = {1.0f, 1.0f};
            float rotation = 1.0f;

            if (registry.all_of<Position>(entity)) {
                position = registry.get<Position>(entity).position;
            }

            if (registry.all_of<Rotation>(entity)) {
                rotation = registry.get<Rotation>(entity).angle;
            }

            if (registry.all_of<Scale>(entity)) {
                scale = registry.get<Scale>(entity).scale;
            }

            float sin = std::sin(glm::radians(rotation));
            float cos = std::cos(glm::radians(rotation));

            transform.position = position;
            transform.matrix = glm::mat2{
                glm::vec2{cos * scale.x, -sin * scale.y},
                glm::vec2{sin * scale.x, cos * scale.y},
            };
        }
    }
}