#pragma once

#include <app/window.hpp>

#include <entt/entt.hpp>

namespace engine::components {
    struct Speed {
        float speed = 1.0f;
    };

    struct PositionController {
        app::Key forwardBinding;
        app::Key backwardBinding;
        app::Key leftBinding;
        app::Key rightBinding;
    };
}