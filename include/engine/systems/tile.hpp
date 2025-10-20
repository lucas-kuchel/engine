#pragma once

#include <renderer/buffer.hpp>
#include <renderer/command_buffer.hpp>

#include <entt/entt.hpp>

namespace engine::systems {
    void createTileMeshes(entt::registry& registry, renderer::Device& device, renderer::CommandBuffer& commandBuffer, renderer::Buffer& stagingBuffer, std::uint64_t& stagingBufferOffset);
    void updateTileMeshes(entt::registry& registry, renderer::CommandBuffer& commandBuffer, renderer::Buffer& stagingBuffer, std::uint64_t& stagingBufferOffset);
    void destroyTileMeshes(entt::registry& registry);
}