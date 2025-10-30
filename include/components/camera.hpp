#pragma once

#include <entt/entt.hpp>
#include <glm/glm.hpp>

namespace components {
    enum class CameraMode {
        FOLLOW,
        LOCKED,
    };

    struct CameraData {
        glm::mat4 projection = {1.0f};
        glm::mat4 view = {1.0f};
    };

    struct Camera {
        float size = 1.0f;
        float near = 0.1f;
        float far = 100.0f;

        CameraMode mode = CameraMode::FOLLOW;
    };

    struct CameraTarget {
        entt::entity target;
    };

    struct CurrentCameraTag {
    };

    struct CameraFollowMouseTag {
    };

    struct CameraSizeAnimator {
        float timeElapsed = 0.0f;
        float duration = 1.0f;

        float targetSize = 1.0f;
        float startSize = 1.0f;
    };

    struct CameraPositionAnimator {
        float timeElapsed = 0.0f;
        float duration = 1.0f;

        glm::vec2 targetPosition = {0.0f, 0.0f};
        glm::vec2 startPosition = {0.0f, 0.0f};
    };
}