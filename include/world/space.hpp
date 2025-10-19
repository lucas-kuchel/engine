#pragma once

#include <optional>
#include <string>

#include <glm/glm.hpp>

namespace engine::components {
    enum class CameraMode : int;
}

namespace world {
    struct Space {
        std::string name;

        struct Bounds {
            glm::vec2 position;
            glm::vec2 extent;
        } bounds;

        struct Camera {
            engine::components::CameraMode mode;

            std::optional<glm::vec2> position;

            float scale;

        } camera;
    };
}