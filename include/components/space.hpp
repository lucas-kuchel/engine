#pragma once

#include <optional>
#include <string>

#include <glm/glm.hpp>

namespace components {
    enum class CameraMode : int;

    struct Space {
        std::string name;

        struct Bounds {
            glm::vec2 position;
            glm::vec2 extent;
        } bounds;

        struct Camera {
            CameraMode mode;

            std::optional<glm::vec2> position;

            float size;

        } camera;

        struct Physics {
            float staticFriction = 1.0f;
            float kineticFriction = 1.0f;
        } physics;
    };
}