#pragma once

#include <app/window.hpp>

namespace game {
    struct MovableBody;

    struct Controller {
        app::Key leftBinding;
        app::Key rightBinding;
        app::Key jumpBinding;
        app::Key sprintBinding;
    };

    void halt(MovableBody& body, Controller& controller, app::WindowKeyReleasedEventInfo& eventInfo);
}