#include <game/controllers.hpp>
#include <game/transforms.hpp>

namespace game {
    void updatePositionControllers(entt::registry& registry, std::span<bool> keymap) {
        for (auto& entity : registry.view<Speed, PositionController, Acceleration>()) {
            auto& speed = registry.get<Speed>(entity);
            auto& controller = registry.get<PositionController>(entity);
            auto& acceleration = registry.get<Acceleration>(entity);

            if (keymap[static_cast<std::size_t>(controller.forwardBinding)]) {
                acceleration.acceleration.y -= speed.speed;
            }

            if (keymap[static_cast<std::size_t>(controller.backwardBinding)]) {
                acceleration.acceleration.y += speed.speed;
            }

            if (keymap[static_cast<std::size_t>(controller.leftBinding)]) {
                acceleration.acceleration.x -= speed.speed;
            }

            if (keymap[static_cast<std::size_t>(controller.rightBinding)]) {
                acceleration.acceleration.x += speed.speed;
            }
        }
    }
}