#include <components/controllers.hpp>
#include <components/entity_tags.hpp>
#include <components/transforms.hpp>

#include <systems/entities.hpp>

#include <engine/engine.hpp>

void systems::entities::createEntities(engine::Engine& engine) {
    using namespace components;

    auto& registry = engine.getRegistry();
    auto& tilePool = engine.getEntityTilePool();
    auto view = registry.view<TileProxy, EntityTag>();

    for (auto [entity, proxy] : view.each()) {
        auto& position = registry.emplace_or_replace<Position>(entity);
        auto& scale = registry.emplace_or_replace<Scale>(entity);
        auto& instance = tilePool.get(proxy);

        position.position = instance.transform.position;
        scale.scale = instance.transform.scale;
    }
}

void systems::entities::updateControllers(engine::Engine& engine) {
    using namespace components;

    auto& registry = engine.getRegistry();
    auto& inputManager = engine.getInputManager();
    auto view = registry.view<Acceleration, PositionController, Speed, EntityTag, CurrentEntityTag>();

    for (auto [entity, acceleration, controller, speed] : view.each()) {
        float finalAngle = 0.0f;
        float finalSpeed = 0.0f;

        if (inputManager.held(controller.forwardBinding)) {
            finalSpeed = speed.speed;
            finalAngle = (finalAngle + glm::radians(0.0f)) * 0.5f;
        }

        if (inputManager.held(controller.backwardBinding)) {
            finalSpeed = speed.speed;
            finalAngle = (finalAngle + glm::radians(90.0f)) * 0.5f;
        }

        if (inputManager.held(controller.leftBinding)) {
            finalSpeed = speed.speed;
            finalAngle = (finalAngle + glm::radians(180.0f)) * 0.5f;
        }

        if (inputManager.held(controller.rightBinding)) {
            finalSpeed = speed.speed;
            finalAngle = (finalAngle + glm::radians(270.0f)) * 0.5f;
        }

        glm::mat2 rotation = {
            glm::vec2{std::cos(finalAngle), -std::sin(finalAngle)},
            glm::vec2{std::sin(finalAngle), std::cos(finalAngle)},
        };

        acceleration.acceleration += rotation * glm::vec2{0.0f, finalSpeed};
    }
}