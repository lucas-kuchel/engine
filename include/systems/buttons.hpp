#pragma once

#include <entt/entt.hpp>

namespace engine {
    class Engine;
}

namespace systems {
    class UI {
        static void createButtons(engine::Engine& engine);
        static void testButtons(engine::Engine& engine);
        static void animateButtons(engine::Engine& engine);
    };
}