#pragma once

#include <app/window.hpp>

namespace game {
    struct Controller {
        app::Key leftBinding;
        app::Key rightBinding;
        app::Key jumpBinding;
        app::Key sprintBinding;
    };
}