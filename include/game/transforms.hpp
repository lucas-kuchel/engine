#pragma once

#include <entt/entt.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace game {
    struct Speed {
        float speed = 1.0f;
    };

    struct Position {
        glm::vec3 position = {0.0f, 0.0f, 0.0f};
    };

    struct Velocity {
        glm::vec3 velocity = {0.0f, 0.0f, 0.0f};
    };

    struct Acceleration {
        glm::vec3 acceleration = {0.0f, 0.0f, 0.0f};
    };

    struct Rotation {
        glm::vec3 rotation = {0.0f, 0.0f, 0.0f};
    };

    struct Scale {
        glm::vec3 scale = {1.0f, 1.0f, 1.0f};
    };

    struct Shear {
        glm::vec3 shear = {0.0f, 0.0f, 0.0f};
    };

    struct Transform {
        glm::mat4 matrix = {1.0f};
    };

    void integrate(entt::registry& registry, float deltaTime);
    void transform(entt::registry& registry);
}