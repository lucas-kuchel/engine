#include <game/transforms.hpp>

#include <glm/gtc/quaternion.hpp>

namespace game {
    void integrate(entt::registry& registry, float deltaTime) {
        for (auto& entity : registry.view<Velocity, Acceleration, Position>()) {
            auto& velocity = registry.get<Velocity>(entity);
            auto& acceleration = registry.get<Acceleration>(entity);
            auto& position = registry.get<Position>(entity);

            velocity.velocity += acceleration.acceleration * deltaTime;
            position.position += velocity.velocity * deltaTime;

            acceleration.acceleration = {0.0f, 0.0f, 0.0f};
        }
    }

    void transform(entt::registry& registry) {
        for (auto entity : registry.view<Transform>()) {
            auto& transform = registry.get<Transform>(entity);

            transform.matrix = {1.0f};

            if (registry.all_of<Scale>(entity)) {
                auto& scale = registry.get<Scale>(entity);

                transform.matrix = glm::scale(transform.matrix, scale.scale);
            }

            if (registry.all_of<Shear>(entity)) {
                auto& shear = registry.get<Shear>(entity);

                if (glm::all(glm::notEqual(shear.shear, glm::vec3{0.0f}))) {
                    glm::mat4 shearMatrix = {1.0f};

                    shearMatrix[1][0] = shear.shear.x;
                    shearMatrix[2][0] = shear.shear.y;
                    shearMatrix[2][1] = shear.shear.z;

                    transform.matrix *= shearMatrix;
                }
            }

            if (registry.all_of<Rotation>(entity)) {
                auto& rotation = registry.get<Rotation>(entity);

                transform.matrix = glm::rotate(transform.matrix, glm::radians(rotation.rotation.z), glm::vec3{0.0f, 0.0f, 1.0f});
                transform.matrix = glm::rotate(transform.matrix, glm::radians(rotation.rotation.y), glm::vec3{0.0f, 1.0f, 0.0f});
                transform.matrix = glm::rotate(transform.matrix, glm::radians(rotation.rotation.x), glm::vec3{1.0f, 0.0f, 0.0f});
            }

            if (registry.all_of<Position>(entity)) {
                auto& position = registry.get<Position>(entity);

                transform.matrix = glm::translate(glm::mat4{1.0f}, position.position) * transform.matrix;
            }
        }
    }
}