#pragma once

#include <entt/entt.hpp>
#include <glm/glm.hpp>

namespace engine::components {
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
        float angle = 0.0f;
    };

    struct Scale {
        glm::vec2 scale = {1.0f, 1.0f};
    };

    struct alignas(32) TransformUploadData {
        glm::vec4 position;
        glm::mat2 model;
    };
}