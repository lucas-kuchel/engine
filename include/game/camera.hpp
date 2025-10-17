#pragma once

#include <renderer/buffer.hpp>
#include <renderer/command_buffer.hpp>
#include <renderer/device.hpp>

#include <entt/entt.hpp>
#include <glm/glm.hpp>

namespace game {
    struct Camera {
        float ease = 1.0f;
        float fov = 60.0f;
        float near = 0.1f;
        float far = 100.0f;
        float scale = 1.0f;

        glm::uvec2 extent = {0u, 0u};
        glm::mat4 projection = {1.0};
        glm::mat4 view = {1.0};
    };

    void updateCameras(entt::registry& registry);

    void createCameraBuffer(renderer::Device& device, renderer::Buffer& cameraBuffer, std::size_t size);
    void updateCameraBuffer(renderer::Buffer& cameraBuffer, std::span<std::uint8_t> data, renderer::CommandBuffer& commandBuffer, renderer::Buffer& stagingBuffer, std::size_t stagingBufferOffset);
    void deleteCameraBuffer(renderer::Buffer& buffer);
}