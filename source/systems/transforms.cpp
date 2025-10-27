#include <components/camera.hpp>
#include <components/entity_tags.hpp>
#include <components/proxy.hpp>
#include <components/tile.hpp>
#include <components/transforms.hpp>
#include <systems/transforms.hpp>

void systems::integrateFriction(entt::registry& registry, components::World& world, float deltaTime) {
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

        if (glm::length(velocity) < velocityEpsilon) {
            velocity = {0.0f, 0.0f};
            continue;
        }

        float tractionFactor = glm::mix(minTractionScale, maxTractionScale, glm::clamp(kineticFriction / (staticFriction + 1e-6f), 0.0f, 1.0f));
        float frictionAccel = kineticFriction * normalForce;
        float decelAmount = frictionAccel * deltaTime;

        bool isAcceleratingX = std::abs(acceleration.x) > std::abs((velocity.x - lastVelocity.x) / std::max(deltaTime, 1e-6f));
        bool isAcceleratingY = std::abs(acceleration.y) > std::abs((velocity.y - lastVelocity.y) / std::max(deltaTime, 1e-6f));

        // Hybrid logic: if both axes accelerating or decelerating, treat as full vector
        if ((isAcceleratingX && isAcceleratingY) || (!isAcceleratingX && !isAcceleratingY)) {
            if (isAcceleratingX && isAcceleratingY) {
                // Apply traction as full vector
                velocity += acceleration * tractionFactor * deltaTime;
            }
            else {
                // Apply friction as vector
                float speed = glm::length(velocity);
                if (speed <= decelAmount) {
                    velocity = {0.0f, 0.0f};
                }
                else {
                    velocity -= (velocity / speed) * decelAmount;
                }
            }
        }
        else {
            // Per-axis handling for mixed cases
            if (isAcceleratingX)
                velocity.x += acceleration.x * tractionFactor * deltaTime;
            else {
                if (std::abs(velocity.x) <= decelAmount)
                    velocity.x = 0.0f;
                else
                    velocity.x -= glm::sign(velocity.x) * decelAmount;
            }

            if (isAcceleratingY)
                velocity.y += acceleration.y * tractionFactor * deltaTime;
            else {
                if (std::abs(velocity.y) <= decelAmount)
                    velocity.y = 0.0f;
                else
                    velocity.y -= glm::sign(velocity.y) * decelAmount;
            }
        }
    }
}

void systems::integrateMovements(entt::registry& registry, float deltaTime) {
    for (auto& entity : registry.view<components::Velocity, components::Acceleration, components::Position>()) {
        auto& velocity = registry.get<components::Velocity>(entity);
        auto& acceleration = registry.get<components::Acceleration>(entity);
        auto& position = registry.get<components::Position>(entity);

        velocity.velocity += acceleration.acceleration * deltaTime;
        acceleration.acceleration = {0.0f, 0.0f};
        position.position += velocity.velocity * deltaTime;
    }
}

void systems::transformInstances(entt::registry& registry, std::span<components::TileInstance> instances) {
    for (auto& entity : registry.view<components::Proxy<components::TileInstance>, components::Position, components::Scale>()) {
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
