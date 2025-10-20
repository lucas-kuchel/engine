#include <engine/components/controllers.hpp>
#include <engine/components/transforms.hpp>
#include <engine/systems/controllers.hpp>

void engine::systems::updatePositionControllers(entt::registry& registry, std::span<bool> keymap) {
    for (auto& entity : registry.view<components::Speed, components::PositionController, components::Acceleration>()) {
        auto& speed = registry.get<components::Speed>(entity);
        auto& controller = registry.get<components::PositionController>(entity);
        auto& acceleration = registry.get<components::Acceleration>(entity);

        if (keymap[static_cast<std::uint64_t>(controller.forwardBinding)]) {
            acceleration.acceleration.y -= speed.speed;
        }

        if (keymap[static_cast<std::uint64_t>(controller.backwardBinding)]) {
            acceleration.acceleration.y += speed.speed;
        }

        if (keymap[static_cast<std::uint64_t>(controller.leftBinding)]) {
            acceleration.acceleration.x -= speed.speed;
        }

        if (keymap[static_cast<std::uint64_t>(controller.rightBinding)]) {
            acceleration.acceleration.x += speed.speed;
        }
    }
}