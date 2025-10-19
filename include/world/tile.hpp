#pragma once

#include <renderer/buffer.hpp>
#include <renderer/command_buffer.hpp>

#include <engine/components/transforms.hpp>

#include <entt/entt.hpp>
#include <glm/glm.hpp>

namespace world {
    struct Tile {
        engine::components::TransformUploadData transform;

        struct alignas(32) Texture {
            struct Sample {
                glm::vec2 position;
                glm::vec2 extent;
            } sample;

            glm::vec2 offset;
            glm::vec2 scale;
        } texture;
    };

    struct TileMesh {
        renderer::Buffer vertexBuffer;
        renderer::Buffer instanceBuffer;
    };

    void createTileMesh(std::size_t tileCount, TileMesh& mesh, renderer::Device& device, renderer::Buffer& stagingBuffer, renderer::CommandBuffer& commandBuffer, std::size_t& stagingBufferOffset);
    void updateTileMesh(std::size_t tileCount, TileMesh& mesh, entt::registry& registry, renderer::Buffer& stagingBuffer, renderer::CommandBuffer& commandBuffer, std::size_t& stagingBufferOffset);
    void destroyTileMesh(TileMesh& mesh);
}