#pragma once

#include <data/unique.hpp>

#include <app/window.hpp>

#include <game/physics.hpp>

#include <renderer/resources/buffer.hpp>

#include <renderer/commands/buffer.hpp>

#include <glm/glm.hpp>

namespace game {
    struct TileVertex {
        glm::vec2 position;
        glm::vec2 texCoord;
    };

    struct TileInstance {
        glm::vec3 position = {0.0f, 0.0f, 0.0f};
        glm::vec2 scale = {1.0f, 1.0f};
        glm::vec2 texOffset = {0.0f, 0.0f};
    };

    struct TileMesh {
        data::Unique<renderer::Buffer> vertexBuffer;
        data::Unique<renderer::Buffer> instanceBuffer;
    };

    struct Map {
        struct Physics {
            float gravity = 0.0f;
            float airResistance = 0.0f;
        } physics;

        std::vector<BoxCollider> colliders;
        std::vector<TileInstance> tiles;
    };

    void createMap(TileMesh& mesh, Map& map, renderer::Device& device, renderer::Buffer& stagingBuffer, std::uint64_t& stagingBufferOffset, renderer::CommandBuffer& transferBuffer);
    void updateMap(TileMesh& mesh, Map& map, renderer::Buffer& stagingBuffer, std::uint64_t& stagingBufferOffset, renderer::CommandBuffer& transferBuffer);
    void renderMap(TileMesh& mesh, Map& map, renderer::CommandBuffer& commandBuffer);

    void loadMapFromFile(Map& map, const std::string& path);

    void resolveMapCollisions(const Map& map, std::span<MovableBody> bodies, std::span<BoxCollider> colliders, std::span<CollisionResult> results);
}