#pragma once

#include <entt/entt.hpp>

namespace engine::components {
    struct CollisionResult {
        entt::entity collider;
    };
}