#pragma once

#include <string>

#include <entt/entt.hpp>

namespace game {
    void loadMap(entt::registry& registry, const std::string& filepath, std::size_t& tileCount);
}