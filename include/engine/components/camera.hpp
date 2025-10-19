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

    struct alignas(128) CameraUploadData {
        glm::mat4 projection = {1.0f};
        glm::mat4 view = {1.0f};
    };
}