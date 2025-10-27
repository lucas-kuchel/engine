#pragma once

#include <renderer/buffer.hpp>

#include <glm/glm.hpp>

namespace components {
    enum class CameraMode {
        FOLLOW,
        LOCKED,
    };

    struct CameraBuffer {
        renderer::Buffer buffer;
    };

    struct Bounds {
        glm::vec2 scale;
    };

    struct Camera {
        float scale = 1.0f;
        float near = 0.1f;
        float far = 100.0f;

        CameraMode mode = CameraMode::FOLLOW;
    };

    struct CameraScaleAnimator {
        float timeElapsed = 0.0f;
        float duration = 1.0f;

        float targetScale = 1.0f;
        float startScale = 1.0f;
    };

    struct CameraPositionAnimator {
        float timeElapsed = 0.0f;
        float duration = 1.0f;

        glm::vec2 targetPosition = {1.0f, 1.0f};
        glm::vec2 startPosition = {1.0f, 1.0f};
    };

    struct alignas(128) CameraUploadData {
        glm::mat4 projection = {1.0f};
        glm::mat4 view = {1.0f};
    };
}