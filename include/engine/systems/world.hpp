#pragma once

#include <renderer/buffer.hpp>
#include <renderer/command_buffer.hpp>

#include <entt/entt.hpp>

namespace engine {
    class Engine;
    class EngineAPI;
}

namespace engine::systems {
    void loadWorlds(entt::registry& registry, Engine& engine);
    void checkTriggers(entt::registry& registry);
    void testCollisions(entt::registry& registry);
    void performTriggers(entt::registry& registry, Engine& engine, EngineAPI& api, float deltaTime);
}