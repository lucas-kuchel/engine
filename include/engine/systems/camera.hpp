#pragma once

#include <renderer/buffer.hpp>
#include <renderer/command_buffer.hpp>
#include <renderer/device.hpp>

#include <entt/entt.hpp>

namespace engine::systems {
    void updateCameraScales(entt::registry& registry, glm::vec2 scale);
    void animateCameras(entt::registry& registry, float deltaTime);
    void createCameras(entt::registry& registry, renderer::Device& device);
    void updateCameras(entt::registry& registry, renderer::Buffer& stagingBuffer, renderer::CommandBuffer& commandBuffer, std::uint64_t& stagingBufferOffset);
    void destroyCameras(entt::registry& registry);
}