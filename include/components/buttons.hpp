#pragma once

#include <entt/entt.hpp>
#include <glm/glm.hpp>

namespace components {
    struct Button {
        std::array<entt::entity, 9> tiles;
    };

    struct ButtonAnimator {
        glm::vec2 originalCentre;
        glm::vec2 originalScale;
        glm::vec2 targetScale;

        float duration = 0.18f;
        float elapsed = 0.0f;

        std::array<glm::vec2, 9> tileCenters;
        std::array<glm::vec2, 9> tileScales;

        bool pulseThenReturn = false;
    };

    struct HoverState {
        bool hovering = false;
    };
}