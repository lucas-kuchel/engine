#pragma once

#include <renderer/buffer.hpp>

#include <glm/glm.hpp>

namespace engine::components {
    enum class CameraMode {
        FOLLOW,
        LOCKED,
    };

    struct CameraBuffer {
        renderer::Buffer buffer;
    };

    struct Camera {
        float scale = 1.0f;
        float near = 0.1f;
        float far = 100.0f;

        CameraMode mode = CameraMode::FOLLOW;
    };

    struct CameraAnimator {
        float timeElapsed = 0.0f;
        float duration = 1.0f;

        float targetScale = 1.0f;
        float lastScale = 1.0f;
        float startScale = 1.0f;

        glm::vec3 startPosition = {0.0f, 0.0f, 0.0f};
        glm::vec3 lastPosition = {0.0f, 0.0f, 0.0f};
        glm::vec3 targetPosition = {0.0f, 0.0f, 0.0f};
    };

    struct alignas(128) CameraUploadData {
        glm::mat4 projection = {1.0f};
        glm::mat4 view = {1.0f};
    };
}