#pragma once

#include <entt/entt.hpp>

#include <optional>
#include <string>
#include <vector>

namespace components {
    struct Trigger {
        struct Event {
            entt::entity action = entt::null;
            std::vector<std::optional<std::string>> parameters;
        };

        bool collideTriggered = false;
        bool separateTriggered = false;

        std::vector<Event> onCollide;
        std::vector<Event> onSeparate;
    };
}