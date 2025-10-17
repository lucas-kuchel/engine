#pragma once

#include <entt/entt.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace game {
    struct Speed {
        float speed = 1.0f;
    };

    struct Position {
        glm::vec3 position;
    };

    struct Velocity {
        glm::vec3 velocity;
    };

    struct Acceleration {
        glm::vec3 acceleration;
    };

    struct Rotation {
        glm::vec3 rotation;
    };

    struct Scale {
        glm::vec3 scale;
    };

    struct Shear {
        glm::vec3 reference;
        glm::vec3 shear;
    };

    struct Transform {
        glm::mat4 matrix;
    };

    struct PositionTracker {
        glm::vec3* position = nullptr;
    };

    void integrate(entt::registry& registry, float deltaTime);
    void updateScales(entt::registry& registry);
    void updateShears(entt::registry& registry);
    void updateRotations(entt::registry& registry);
    void updatePositions(entt::registry& registry);
}