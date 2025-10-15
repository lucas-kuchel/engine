#pragma once

#include <glm/glm.hpp>

namespace game {
    struct MovableBody {
        glm::vec3 position = {0.0f, 0.0f, 0.0f};
        glm::vec3 velocity = {0.0f, 0.0f, 0.0f};
        glm::vec3 acceleration = {0.0f, 0.0f, 0.0f};
    };

    struct Collider {
        glm::vec2 position = {0.0f, 0.0f};
        glm::vec2 scale = {1.0f, 1.0f};

        float rotation = 0.0f;
    };

    struct CollisionResult {
        bool collided = false;

        glm::vec2 penetration = {0.0f, 0.0f};
        glm::vec2 normal = {0.0f, 0.0f};

        bool left = false;
        bool right = false;
        bool top = false;
        bool bottom = false;
    };

    void updateMovement(MovableBody& body, float deltaTime, float gravity, float friction, float airResistance);

    void testCollisionOBB(const Collider& a, const Collider& b, CollisionResult& result);
}