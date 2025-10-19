#pragma once

#include <optional>
#include <string>
#include <vector>

#include <glm/glm.hpp>

namespace world {
    struct Trigger {
        struct Bounds {
            glm::vec2 position;
            glm::vec2 extent;
        } bounds;

        struct Event {
            std::string action;
            std::vector<std::optional<std::string>> parameters;
        };

        std::vector<Event> onEnter;
        std::vector<Event> onExit;
    };
}