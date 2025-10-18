#pragma once

#include <renderer/buffer.hpp>
#include <renderer/command_buffer.hpp>

#include <entt/entt.hpp>
#include <glm/glm.hpp>

namespace game {
    struct alignas(16) MeshTexture {
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

    struct alignas(16) MeshVertex {
        glm::vec2 position;
    };

    struct alignas(16) MeshTransform {
        glm::vec3 position = {0.0f, 0.0f, 0.0f};
        glm::mat2 matrix = {1.0f};
    };

    void createMesh(std::size_t tileCount, Mesh& mesh, renderer::Device& device, renderer::Buffer& stagingBuffer, renderer::CommandBuffer& commandBuffer, std::size_t stagingBufferOffset);
    void updateMesh(std::size_t tileCount, Mesh& mesh, entt::registry& registry, renderer::Buffer& stagingBuffer, renderer::CommandBuffer& commandBuffer, std::size_t stagingBufferOffset);
    void destroyMesh(Mesh& mesh);
}