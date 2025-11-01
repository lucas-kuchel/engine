#include <components/camera.hpp>
#include <components/tags.hpp>
#include <components/transforms.hpp>

#include <engine/engine.hpp>

#include <systems/transforms.hpp>

void systems::integrateMovements(engine::Engine& engine) {
    auto& registry = engine.getRegistry();

    auto deltaTime = engine.getDeltaTime();
    auto view = registry.view<components::Velocity, components::Acceleration, components::Position>();

    for (auto [entity, velocity, acceleration, position] : view.each()) {

        velocity.velocity += acceleration.acceleration * deltaTime;
        acceleration.acceleration = {0.0f, 0.0f};
        position.position += velocity.velocity * deltaTime;
    }
}

void systems::transformInstances(engine::Engine& engine, engine::TilePool& tilePool) {
    auto& registry = engine.getRegistry();

    for (auto& entity : registry.view<components::TileProxy, components::Position, components::Scale>()) {
        auto& proxy = registry.get<components::TileProxy>(entity);
        auto& position = registry.get<components::Position>(entity);
        auto& scale = registry.get<components::Scale>(entity);

        if (!tilePool.contains(proxy)) {
            continue;
        }

        auto& tileInstance = tilePool.getInstance(proxy);

        tileInstance.transform.position = {
            position.position.x,
            position.position.y,
        };

        tileInstance.transform.scale = scale.scale;
    }
}
