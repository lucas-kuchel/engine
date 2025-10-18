#pragma once

#include <renderer/buffer.hpp>
#include <renderer/command_buffer.hpp>

#include <entt/entt.hpp>
#include <glm/glm.hpp>

namespace game {
    struct alignas(32) MeshTexture {
        glm::vec2 position = {0.0f, 0.0f};
        glm::vec2 extent = {1.0f, 1.0f};
        glm::vec2 offset = {0.0f, 0.0f};
        glm::vec2 scale = {1.0f, 1.0f};
    };

    struct Mesh {
        renderer::Buffer vertexBuffer;
        renderer::Buffer textureBuffer;
        renderer::Buffer transformBuffer;
    };

    struct MeshVertex {
        glm::vec2 position;
    };

    void createMesh(std::size_t tileCount, Mesh& mesh, renderer::Device& device, renderer::Buffer& stagingBuffer, renderer::CommandBuffer& commandBuffer, std::size_t stagingBufferOffset);
    void updateMesh(std::size_t tileCount, Mesh& mesh, entt::registry& registry, renderer::Buffer& stagingBuffer, renderer::CommandBuffer& commandBuffer, std::size_t stagingBufferOffset);
    void destroyMesh(Mesh& mesh);
}