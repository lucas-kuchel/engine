#include <engine/components/transforms.hpp>
#include <engine/systems/transforms.hpp>

// #include <glm/gtc/matrix_transform.hpp>

void engine::systems::integrateMovements(entt::registry& registry, float deltaTime) {
    for (auto& entity : registry.view<components::Velocity, components::Acceleration>()) {
        auto& velocity = registry.get<components::Velocity>(entity);
        auto& acceleration = registry.get<components::Acceleration>(entity);

        velocity.velocity += acceleration.acceleration * deltaTime;
        acceleration.acceleration = {0.0f, 0.0f, 0.0f};

        if (registry.all_of<components::Position>(entity)) {
            auto& position = registry.get<components::Position>(entity);

            position.position += velocity.velocity * deltaTime;
        }
    }
}

// TODO: move to other systems
// void engine::systems::updateTransforms(entt::registry& registry) {
//     for (auto entity : registry.view<components::Transform>()) {
//         auto& transform = registry.get<components::Transform>(entity);
//
//         glm::vec3 position = {0.0f, 0.0f, 0.0f};
//         glm::vec2 scale = {1.0f, 1.0f};
//         float rotation = 1.0f;
//
//         if (registry.all_of<components::Position>(entity)) {
//             position = registry.get<components::Position>(entity).position;
//         }
//
//         if (registry.all_of<components::Rotation>(entity)) {
//             rotation = registry.get<components::Rotation>(entity).angle;
//         }
//
//         if (registry.all_of<components::Scale>(entity)) {
//             scale = registry.get<components::Scale>(entity).scale;
//         }
//
//         float sin = std::sin(glm::radians(rotation));
//         float cos = std::cos(glm::radians(rotation));
//
//         transform.position = position;
//         transform.model = glm::mat2{
//             glm::vec2{cos * scale.x, -sin * scale.y},
//             glm::vec2{sin * scale.x, cos * scale.y},
//         };
//     }
// }