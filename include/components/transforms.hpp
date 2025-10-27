#pragma once

#include <entt/entt.hpp>
#include <glm/glm.hpp>

namespace components {
    struct Position {
        glm::vec2 position = {0.0f, 0.0f};
    };

    struct Velocity {
        glm::vec2 velocity = {0.0f, 0.0f};
    };

    struct Acceleration {
        glm::vec2 acceleration = {0.0f, 0.0f};
    };

    struct Rotation {
        float angle = 0.0f;
    };

    struct Scale {
        glm::vec2 scale = {1.0f, 1.0f};
    };

    template <typename T>
    struct Last {
        T value;
    };

    template <typename T>
    struct Original {
        T value;
    };
}