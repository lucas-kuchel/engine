#pragma once

#include <renderer/buffer.hpp>
#include <renderer/command_buffer.hpp>
#include <renderer/device.hpp>

#include <entt/entt.hpp>
#include <glm/glm.hpp>

namespace game {
    struct CameraBuffer {
        renderer::Buffer buffer;
    };

    struct Orthographic {
        float left = 0.0f;
        float right = 0.0f;
        float top = 0.0f;
        float bottom = 0.0f;
        float scale = 1.0f;
    };

    struct Projection {
        glm::mat4 matrix = {1.0f};
    };

    struct View {
        glm::mat4 matrix = {1.0f};
    };

    struct Target {
        entt::entity handle;
    };

    struct Camera {
        float near = 0.1f;
        float far = 100.0f;
    };

    void createCameraBuffers(entt::registry& registry, renderer::Device& device);
    void updateCameraBuffers(entt::registry& registry, renderer::Buffer& stagingBuffer, renderer::CommandBuffer& commandBuffer, std::size_t& stagingBufferOffset);
    void destroyCameraBuffers(entt::registry& registry);

    void cameraFollow(entt::registry& registry);
    void updateCameraOrthographics(entt::registry& registry, glm::vec2 extent);
    void updateCameraViews(entt::registry& registry);
}