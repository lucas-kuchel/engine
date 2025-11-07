#pragma once

#include <vulkanite/window/window.hpp>

#include <entt/entt.hpp>

namespace components {
    struct Speed {
        float speed = 1.0f;
    };

    struct PositionController {
        vulkanite::window::Key forwardBinding;
        vulkanite::window::Key backwardBinding;
        vulkanite::window::Key leftBinding;
        vulkanite::window::Key rightBinding;
    };
}