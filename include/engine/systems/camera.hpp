#pragma once

#include <renderer/buffer.hpp>
#include <renderer/command_buffer.hpp>
#include <renderer/device.hpp>

#include <entt/entt.hpp>

namespace engine::systems {
    void createCameras(entt::registry& registry, renderer::Device& device);
    void updateCameras(entt::registry& registry, renderer::Buffer& stagingBuffer, renderer::CommandBuffer& commandBuffer, std::size_t& stagingBufferOffset);
    void destroyCameras(entt::registry& registry);
}