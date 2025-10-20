#pragma once

#include <engine/components/tile.hpp>

#include <entt/entt.hpp>

namespace engine::components {
    struct World {
        std::string path;

        entt::entity defaults;

        std::vector<entt::entity> actions;
        std::vector<entt::entity> spaces;
        std::vector<entt::entity> triggers;

        std::vector<components::Tile> tiles;

        std::optional<entt::entity> currentSpace;
    };
}