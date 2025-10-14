#include <game/physics.hpp>

#include <algorithm>

namespace game {
    void updateMovement(MovableBody& body, float deltaTime, float gravity, float friction, float airResistance) {
        body.acceleration.y -= gravity;
        body.velocity += body.acceleration * deltaTime;

        body.velocity *= 1.0f / (1.0f + airResistance * deltaTime);
        body.velocity.x *= 1.0f / (1.0f + friction * deltaTime);

        const float epsilon = 0.01f;

        if (std::abs(body.velocity.x) < epsilon) {
            body.velocity.x = 0.0f;
        }

        if (std::abs(body.velocity.y) < epsilon) {
            body.velocity.y = 0.0f;
        }

        body.position += body.velocity * deltaTime;
        body.acceleration = {0.0f, 0.0f};
    }

    void testCollisionAABB(const BoxCollider& a, const BoxCollider& b, BoxCollisionResult& result) {
        glm::vec2 aMin = a.position - a.scale * 0.5f;
        glm::vec2 bMin = b.position - b.scale * 0.5f;
        glm::vec2 aMax = a.position + a.scale * 0.5f;
        glm::vec2 bMax = b.position + b.scale * 0.5f;

        float overlapX = std::min(aMax.x, bMax.x) - std::max(aMin.x, bMin.x);
        float overlapY = std::min(aMax.y, bMax.y) - std::max(aMin.y, bMin.y);

        if (overlapX > 0 && overlapY > 0) {
            result.collided = true;
            result.other = &b;

            if (overlapX < overlapY) {
                if (aMin.x < bMin.x) {
                    result.right = true;
                    result.normal = glm::vec2{-1.0f, 0.0f};
                }
                else {
                    result.left = true;
                    result.normal = glm::vec2{1.0f, 0.0f};
                }

                result.penetration = {overlapX, 0.0f};
            }
            else {
                if (aMin.y < bMin.y) {
                    result.top = true;
                    result.normal = glm::vec2{0.0f, -1.0f};
                }
                else {
                    result.bottom = true;
                    result.normal = glm::vec2{0.0f, 1.0f};
                }

                result.penetration = {0.0f, overlapY};
            }
        }
    }
}