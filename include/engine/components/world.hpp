#pragma once

#include <engine/components/defaults.hpp>
#include <engine/components/tile.hpp>

#include <entt/entt.hpp>

namespace engine::components {
    struct World {
        std::string path;

        std::vector<entt::entity> actions;
        std::vector<entt::entity> spaces;
        std::vector<entt::entity> triggers;
        std::vector<entt::entity> tiles;

        Defaults defaults;

        entt::entity currentSpace;
    };
}