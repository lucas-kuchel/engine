#pragma once

#include <entt/entt.hpp>

namespace engine::components {
    struct World {
        std::string path;

        entt::entity defaults;

        std::vector<entt::entity> actions;
        std::vector<entt::entity> spaces;
        std::vector<entt::entity> tiles;
        std::vector<entt::entity> triggers;

        std::optional<entt::entity> currentSpace;

        entt::entity tileMesh;
    };
}