#include <components/controllers.hpp>
#include <components/transforms.hpp>
#include <systems/controllers.hpp>

void systems::updatePositionControllers(entt::registry& registry, std::span<bool> keymap) {
    for (auto& entity : registry.view<components::Speed, components::PositionController, components::Acceleration>()) {
        auto& speed = registry.get<components::Speed>(entity);
        auto& controller = registry.get<components::PositionController>(entity);
        auto& acceleration = registry.get<components::Acceleration>(entity);

        if (keymap[static_cast<std::uint64_t>(controller.forwardBinding)]) {
            acceleration.acceleration.y += speed.speed;
        }

        if (keymap[static_cast<std::uint64_t>(controller.backwardBinding)]) {
            acceleration.acceleration.y -= speed.speed;
        }

        if (keymap[static_cast<std::uint64_t>(controller.leftBinding)]) {
            acceleration.acceleration.x -= speed.speed;
        }

        if (keymap[static_cast<std::uint64_t>(controller.rightBinding)]) {
            acceleration.acceleration.x += speed.speed;
        }
    }
}

void systems::clampSpeeds(entt::registry& registry) {
    for (auto& entity : registry.view<components::Speed, components::Velocity>()) {
        auto& velocity = registry.get<components::Velocity>(entity).velocity;
        auto& maxSpeed = registry.get<components::Speed>(entity).speed;

        float currentSpeed = glm::length(velocity);

        if (currentSpeed > maxSpeed) {
            velocity = (velocity / currentSpeed) * maxSpeed;
        }
    }
}