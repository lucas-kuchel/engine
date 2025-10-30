#include <components/controllers.hpp>
#include <components/entity_tags.hpp>
#include <components/transforms.hpp>

#include <systems/entities.hpp>

#include <engine/engine.hpp>

void systems::entities::createEntities(engine::Engine& engine) {
    using namespace components;

    auto& registry = engine.getRegistry();
    auto& tilePool = engine.getEntityTilePool();
    auto view = registry.view<engine::TileProxy, EntityTag>();

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
        glm::vec2 direction = {0.0f, 0.0f};

        if (inputManager.held(controller.forwardBinding)) {
            direction.y += 1.0f;
        }
        if (inputManager.held(controller.backwardBinding)) {
            direction.y -= 1.0f;
        }
        if (inputManager.held(controller.leftBinding)) {
            direction.x -= 1.0f;
        }
        if (inputManager.held(controller.rightBinding)) {
            direction.x += 1.0f;
        }

        if (glm::length(direction) > 0.0f) {
            direction = glm::normalize(direction);
            acceleration.acceleration += direction * speed.speed;
        }
    }
}
