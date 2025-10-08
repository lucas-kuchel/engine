#pragma once

#include <data/unique.hpp>

#include <app/window.hpp>

#include <renderer/resources/buffer.hpp>

#include <renderer/commands/buffer.hpp>

#include <renderer/swapchain.hpp>

#include <glm/glm.hpp>

namespace game {
    struct Camera {
        float ease = 1.0f;
        float rotation = 0.0f;
        float scale = 20.0f;

        glm::vec2 position = {0.0f, 0.0f};
        glm::mat4 projection = {1.0f};
        glm::mat4 view = {1.0f};

        glm::uvec2 extent = {0, 0};

        data::Unique<renderer::Buffer> uniformBuffer;
    };

    void createCamera(Camera& camera, renderer::Device& device, renderer::Buffer& stagingBuffer, std::uint64_t& stagingBufferOffset, renderer::CommandBuffer& transferBuffer);
    void updateCamera(Camera& camera, renderer::Buffer& stagingBuffer, std::uint64_t& stagingBufferOffset, renderer::CommandBuffer& transferBuffer);

    void easeCameraTowards(Camera& camera, glm::vec2& position, float deltaTime);
}