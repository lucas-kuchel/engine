#pragma once

#include <app/window.hpp>

namespace game {
    struct PositionController {
        app::Key forwardBinding;
        app::Key backwardBinding;
        app::Key leftBinding;
        app::Key rightBinding;
    };
}