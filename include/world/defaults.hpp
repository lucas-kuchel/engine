#pragma once

#include <optional>
#include <string>

#include <glm/glm.hpp>

namespace engine::components {
    enum class CameraMode : int;
}

namespace world {
    struct Defaults {
        std::string name;

        struct Camera {
            engine::components::CameraMode mode;

            std::optional<glm::vec2> position;

            float scale;

        } camera;
    };
}