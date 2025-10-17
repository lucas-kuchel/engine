#pragma once

#include <renderer/buffer.hpp>
#include <renderer/command_buffer.hpp>

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

    void createMesh(renderer::Device& device, Mesh& mesh, std::size_t instanceCount);
    void updateMesh(Mesh& mesh, std::size_t instanceCount, std::span<std::uint8_t> data, renderer::CommandBuffer& commandBuffer, renderer::Buffer& stagingBuffer, std::size_t stagingBufferOffset);
    void deleteMesh(Mesh& mesh);
}