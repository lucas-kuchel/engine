#include <engine/components/camera.hpp>
#include <engine/components/entity_tags.hpp>
#include <engine/components/proxy.hpp>
#include <engine/components/tile.hpp>
#include <engine/components/transforms.hpp>
#include <engine/systems/transforms.hpp>

void engine::systems::integrateFriction(entt::registry& registry, components::World& world, float deltaTime) {
    const float normalForce = 1.0f;
    const float staticFriction = world.currentState.physics.staticFriction;
    const float kineticFriction = world.currentState.physics.kineticFriction;
    const float velocityEpsilon = 1e-4f;

    const float minTractionScale = 0.2f;
    const float maxTractionScale = 1.0f;

    for (auto& entity : registry.view<components::ApplyFrictionTag, components::Velocity, components::Acceleration, components::Last<components::Velocity>>()) {
        auto& velocity = registry.get<components::Velocity>(entity).velocity;
        auto& lastVelocity = registry.get<components::Last<components::Velocity>>(entity).value.velocity;
        auto& acceleration = registry.get<components::Acceleration>(entity).acceleration;

        float speed = glm::length(velocity);

        if (speed < velocityEpsilon) {
            velocity = {0.0f, 0.0f};
            continue;
        }

        float accelMagnitude = glm::length(acceleration);
        float lastAccelMagnitude = glm::length(lastVelocity - velocity) / std::max(deltaTime, 1e-6f);
        bool isAccelerating = accelMagnitude > lastAccelMagnitude;

        if (isAccelerating) {
            float tractionFactor = glm::mix(minTractionScale, maxTractionScale, glm::clamp(kineticFriction / (staticFriction + 1e-6f), 0.0f, 1.0f));

            acceleration *= tractionFactor;
            velocity += acceleration * deltaTime;
        }
        else {
            float frictionAccel = kineticFriction * normalForce;
            float decelAmount = frictionAccel * deltaTime;

            if (decelAmount >= speed) {
                velocity = {0.0f, 0.0f};
            }
            else {
                velocity -= (velocity / speed) * decelAmount;
            }
        }
    }
}

void engine::systems::integrateMovements(entt::registry& registry, float deltaTime) {
    for (auto& entity : registry.view<components::Velocity, components::Acceleration, components::Position>()) {
        auto& velocity = registry.get<components::Velocity>(entity);
        auto& acceleration = registry.get<components::Acceleration>(entity);
        auto& position = registry.get<components::Position>(entity);

        velocity.velocity += acceleration.acceleration * deltaTime;
        acceleration.acceleration = {0.0f, 0.0f};
        position.position += velocity.velocity * deltaTime;
    }
}

void engine::systems::transformInstances(entt::registry& registry, std::span<components::TileInstance> instances) {
    for (auto& entity : registry.view<components::Proxy<components::TileInstance>, components::Position, components::Scale>(entt::exclude<components::TileTag>)) {
        auto& proxy = registry.get<components::Proxy<components::TileInstance>>(entity);
        auto& position = registry.get<components::Position>(entity);
        auto& scale = registry.get<components::Scale>(entity);
        auto& tileInstance = instances[proxy.index];

        tileInstance.transform.position = {
            position.position.x,
            position.position.y,
            tileInstance.transform.position.z,
        };

        tileInstance.transform.scale = scale.scale;
    }
}
