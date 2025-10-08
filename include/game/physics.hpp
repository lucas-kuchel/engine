#pragma once

#include <data/references.hpp>

#include <glm/glm.hpp>

namespace game {
    struct MovableBody {
        glm::vec2 position = {0.0f, 0.0f};
        glm::vec2 velocity = {0.0f, 0.0f};
        glm::vec2 acceleration = {0.0f, 0.0f};
    };

    struct BoxCollider {
        struct Physics {
            float friction = 0.0f;
        } physics;

        glm::vec2 position = {0.0f, 0.0f};
        glm::vec2 extent = {0.0f, 0.0f};
    };

    struct CollisionResult {
        bool collided = false;

        glm::vec2 penetration = {0.0f, 0.0f};
        glm::vec2 normal = {0.0f, 0.0f};

        data::NullableRef<const BoxCollider> other = nullptr;
    };

    void updateMovement(MovableBody& body, float deltaTime, float gravity, float friction, float airResistance);

    void testCollisionAABB(const BoxCollider& a, const BoxCollider& b, CollisionResult& result);
}