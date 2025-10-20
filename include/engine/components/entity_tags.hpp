#pragma once

#include <entt/entt.hpp>

namespace engine::components {
    struct StaticTileTag {
    };

    struct ActiveCharacterTag {
    };

    struct ActiveCameraTag {
    };

    struct ActiveWorldTag {
    };

    struct CanActivateTriggerTag {
        entt::entity currentTrigger = entt::null;
    };
}