#pragma once

#include <app/window.hpp>

#include <game/physics.hpp>

#include <renderer/buffer.hpp>
#include <renderer/command_buffer.hpp>

#include <glm/glm.hpp>

namespace game {
    struct alignas(32) TileInstance {
        glm::vec2 texturePosition = {0.0f, 0.0f};
        glm::vec2 textureExtent = {1.0f, 1.0f};
        glm::vec2 textureOffset = {0.0f, 0.0f};
        glm::vec2 textureScale = {1.0f, 1.0f};
    };

    struct TileMesh {
        renderer::Buffer vertexBuffer;
        renderer::Buffer instanceBuffer;
        renderer::Buffer modelBuffer;
    };

    struct Map {
        TileMesh mesh;

        std::vector<Collider> colliders;
        std::vector<TileInstance> instances;
        std::vector<glm::mat4> models;
    };

    void createMap(Map& map, renderer::Device& device, renderer::Buffer& stagingBuffer, std::uint64_t& stagingBufferOffset, renderer::CommandBuffer& transferBuffer);
    void renderMap(Map& map, renderer::CommandBuffer& commandBuffer);
    void destroyMap(Map& map);

    void loadMapFromFile(Map& map, const std::string& path);

    void resolveMapCollisions(const Map& map, std::span<MovableBody> bodies, std::span<Collider> colliders, std::span<CollisionResult> results);
}