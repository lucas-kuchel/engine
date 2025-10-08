#include <game/physics.hpp>

namespace game {
    void updateMovement(MovableBody& body, float deltaTime, float gravity, float friction, float airResistance) {
        body.acceleration.y -= gravity;
        body.velocity += body.acceleration * deltaTime;

        body.velocity *= 1.0f / (1.0f + airResistance * deltaTime);
        body.velocity.x *= 1.0f / (1.0f + friction * deltaTime);

        body.position += body.velocity * deltaTime;
        body.acceleration = {0.0f, 0.0f};
    }

    void testCollisionAABB(const BoxCollider& a, const BoxCollider& b, CollisionResult& result) {
        glm::vec2 aMin = a.position - a.extent * 0.5f;
        glm::vec2 aMax = a.position + a.extent * 0.5f;
        glm::vec2 bMin = b.position - b.extent * 0.5f;
        glm::vec2 bMax = b.position + b.extent * 0.5f;

        float overlapX = std::min(aMax.x, bMax.x) - std::max(aMin.x, bMin.x);
        float overlapY = std::min(aMax.y, bMax.y) - std::max(aMin.y, bMin.y);

        if (overlapX > 0 && overlapY > 0) {
            result.collided = true;

            if (overlapX < overlapY) {
                result.penetration = {overlapX, 0.0f};
                result.normal = (a.position.x < b.position.x) ? glm::vec2{-1.0f, 0.0f} : glm::vec2{1.0f, 0.0f};
            }
            else {
                result.penetration = {0.0f, overlapY};
                result.normal = (a.position.y < b.position.y) ? glm::vec2{0.0f, -1.0f} : glm::vec2{0.0f, 1.0f};
            }
        }

        result.other = b;
    }
}