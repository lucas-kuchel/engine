#pragma once

#include <components/tile.hpp>
#include <entt/entt.hpp>

namespace components {
    enum class CameraMode : int;

    struct WorldConfig {
        std::string name;
        std::string path;

        struct Camera {
            CameraMode mode;

            std::optional<glm::vec2> position;

            float size;

        } camera;

        struct Physics {
            float staticFriction = 1.0f;
            float kineticFriction = 1.0f;
        } physics;
    };

    struct World {
        std::vector<entt::entity> actions;
        std::vector<entt::entity> spaces;
        std::vector<entt::entity> colliders;
        std::vector<entt::entity> triggers;
        std::vector<entt::entity> tiles;

        std::vector<std::vector<TileProxy>> tileGroups;

        WorldConfig defaultState;
        WorldConfig currentState;
    };
}