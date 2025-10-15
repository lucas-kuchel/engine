#pragma once

#include <app/window.hpp>

#include <renderer/buffer.hpp>
#include <renderer/command_buffer.hpp>

#include <renderer/swapchain.hpp>

#include <glm/glm.hpp>

namespace game {
    struct Camera {
        float ease = 1.0f;
        float fov = 60.0f;
        float near = 0.1f;
        float far = 100.0f;

        glm::vec2 rotation = {-35.0f, 45.0f};
        glm::vec3 orientation = {0.0f, 0.0f, 1.0f};
        glm::vec3 position = {0.0f, 5.0f, 5.0f};
        glm::mat4 projection = {1.0f};
        glm::mat4 view = {1.0f};

        glm::uvec2 extent = {0, 0};

        renderer::Buffer uniformBuffer;
    };

    void createCamera(Camera& camera, renderer::Device& device, renderer::Buffer& stagingBuffer, std::uint64_t& stagingBufferOffset, renderer::CommandBuffer& transferBuffer);
    void updateCamera(Camera& camera, renderer::Buffer& stagingBuffer, std::uint64_t& stagingBufferOffset, renderer::CommandBuffer& transferBuffer);
    void destroyCamera(Camera& camera);

    void easeCameraTowards(Camera& camera, glm::vec3 position, float deltaTime);
}