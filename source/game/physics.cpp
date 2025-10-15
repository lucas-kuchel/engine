#include <game/physics.hpp>

namespace game {
    void updateMovement(MovableBody& body, float deltaTime, float gravity, float, float) {
        body.acceleration.y -= gravity;
        body.velocity += body.acceleration * deltaTime;
        body.position += body.velocity * deltaTime;
        body.acceleration = {0.0f, 0.0f, 0.0f};

        const float epsilon = 0.01f;

        if (std::abs(body.velocity.x) < epsilon) {
            body.velocity.x = 0.0f;
        }

        if (std::abs(body.velocity.y) < epsilon) {
            body.velocity.y = 0.0f;
        }
    }

    void testCollisionOBB(const Collider& a, const Collider& b, CollisionResult& result) {
        // Local axes
        glm::vec2 aAxisX = glm::vec2(std::cos(a.rotation), std::sin(a.rotation));
        glm::vec2 aAxisY = glm::vec2(-std::sin(a.rotation), std::cos(a.rotation));

        glm::vec2 bAxisX = glm::vec2(std::cos(b.rotation), std::sin(b.rotation));
        glm::vec2 bAxisY = glm::vec2(-std::sin(b.rotation), std::cos(b.rotation));

        // Half extents
        glm::vec2 aHalf = a.scale * 0.5f;
        glm::vec2 bHalf = b.scale * 0.5f;

        // Centers
        glm::vec2 aCenter = a.position;
        glm::vec2 bCenter = b.position;

        // Vector from A to B
        glm::vec2 d = bCenter - aCenter;

        // Store minimal overlap info
        float minOverlap = std::numeric_limits<float>::max();
        glm::vec2 smallestAxis;

        auto projectOntoAxis = [](const glm::vec2& axis, const glm::vec2& halfExtents, const glm::vec2& rotX, const glm::vec2& rotY) {
            return halfExtents.x * std::abs(glm::dot(axis, rotX)) + halfExtents.y * std::abs(glm::dot(axis, rotY));
        };

        // Test axes: A's axes
        glm::vec2 axes[4] = {aAxisX, aAxisY, bAxisX, bAxisY};
        for (int i = 0; i < 4; ++i) {
            glm::vec2 axis = axes[i];

            // Project both boxes onto axis
            float projA = projectOntoAxis(axis, aHalf, aAxisX, aAxisY);
            float projB = projectOntoAxis(axis, bHalf, bAxisX, bAxisY);
            float dist = std::abs(glm::dot(d, axis));

            float overlap = projA + projB - dist;
            if (overlap <= 0.0f) {
                result.collided = false;
                return; // Separating axis found -> no collision
            }

            if (overlap < minOverlap) {
                minOverlap = overlap;
                smallestAxis = axis;
            }
        }

        // Collision detected, store minimal penetration
        result.collided = true;
        result.penetration = smallestAxis * minOverlap;

        // Determine normal direction from A to B
        if (glm::dot(d, smallestAxis) < 0.0f)
            result.normal = -smallestAxis;
        else
            result.normal = smallestAxis;

        // Optional: determine sides (top/bottom/left/right) in local space
        // This part is more heuristic; you can rotate the normal into A's local frame
        glm::vec2 localNormal = glm::vec2(
            glm::dot(result.normal, aAxisX),
            glm::dot(result.normal, aAxisY));

        result.left = localNormal.x < 0;
        result.right = localNormal.x > 0;
        result.top = localNormal.y < 0;
        result.bottom = localNormal.y > 0;
    }
}