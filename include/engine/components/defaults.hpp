#pragma once

#include <optional>
#include <string>

#include <glm/glm.hpp>

namespace engine::components {
    enum class CameraMode : int;

    struct Defaults {
        std::string name;

        struct Camera {
            CameraMode mode;

            std::optional<glm::vec2> position;

            float scale;

        } camera;
    };
}