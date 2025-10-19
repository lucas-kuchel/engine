#pragma once

#include <renderer/buffer.hpp>
#include <renderer/command_buffer.hpp>

#include <entt/entt.hpp>

namespace engine::systems {
    void loadWorlds(entt::registry& registry);
}