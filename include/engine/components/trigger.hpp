#pragma once

#include <optional>
#include <string>
#include <vector>

#include <entt/entt.hpp>
#include <glm/glm.hpp>

namespace engine::components {
    struct Trigger {
        struct Bounds {
            glm::vec2 position;
            glm::vec2 extent;
        } bounds;

        struct Event {
            entt::entity action = entt::null;
            std::vector<std::optional<std::string>> parameters;
        };

        bool onEnterTriggered = false;
        bool onExitTriggered = false;

        std::vector<Event> onEnter;
        std::vector<Event> onExit;
    };
}