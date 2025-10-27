#pragma once

#include <entt/entt.hpp>
#include <glm/glm.hpp>

namespace engine::components {
    struct Button {
        std::array<entt::entity, 9> tiles;
    };

    struct ButtonAnimator {
        glm::vec2 originalScale;
        glm::vec2 targetScale;

        glm::vec2 originalPosition;
        glm::vec2 cachedPosition;

        float duration = 0.0f;
        float elapsed = 0.0f;
    };
}