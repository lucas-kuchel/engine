#pragma once

#include <entt/entt.hpp>

namespace engine {
    class Engine;
}

namespace systems::entities {
    void createEntities(engine::Engine& engine);
    void updateControllers(engine::Engine& engine);
}