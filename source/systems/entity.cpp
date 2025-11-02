#include <components/entity.hpp>
#include <components/tags.hpp>
#include <components/transforms.hpp>

#include <systems/entity.hpp>

#include <engine/engine.hpp>

void systems::entities::createEntities(engine::Engine& engine) {
    using namespace components;

    auto& registry = engine.getRegistry();
    auto& tilePool = engine.getEntityTilePool();
    auto view = registry.view<TileProxy, EntityTag>();

    for (auto [entity, proxy] : view.each()) {
        auto& position = registry.emplace_or_replace<Position>(entity);
        auto& scale = registry.emplace_or_replace<Scale>(entity);
        auto& instance = tilePool.getInstance(proxy);

        position.position = engine::screenToWorldSpace(instance.transform.position);
        scale.scale = instance.transform.scale;
    }
}

void systems::entities::sortEntities(engine::Engine& engine) {
    using namespace components;

    auto& registry = engine.getRegistry();
    auto& tilePool = engine.getEntityTilePool();
    auto& worldGenerator = engine.getWorldGenerator();
    auto worldSize = worldGenerator.getWorldSize();
    auto view = registry.view<TileProxy, Position, EntityTag>();

    for (auto [entity, proxy, position] : view.each()) {
        auto& data = tilePool.getData(proxy);

        data.order = static_cast<std::int64_t>((static_cast<float>(worldSize.y) - position.position.y) * static_cast<float>(worldSize.x + worldSize.z - 1) + position.position.x + position.position.z);
    }
}

void systems::entities::updateControllers(engine::Engine& engine) {
    using namespace components;

    auto& registry = engine.getRegistry();
    auto& inputManager = engine.getInputManager();
    auto view = registry.view<Acceleration, PositionController, Speed, EntityTag>();

    for (auto [entity, acceleration, controller, speed] : view.each()) {
        glm::vec3 direction = {0.0f, 0.0f, 0.0f};

        if (inputManager.held(controller.forwardBinding)) {
            direction.x += 0.5f;
            direction.z += 0.5f;
        }
        if (inputManager.held(controller.backwardBinding)) {
            direction.x -= 0.5f;
            direction.z -= 0.5f;
        }
        if (inputManager.held(controller.leftBinding)) {
            direction.x -= 0.5f;
            direction.z += 0.5f;
        }
        if (inputManager.held(controller.rightBinding)) {
            direction.x += 0.5f;
            direction.z -= 0.5f;
        }

        if (glm::length(direction) > 0.0f) {
            direction = glm::normalize(direction);
            acceleration.acceleration += direction * speed.speed;
        }
    }
}
