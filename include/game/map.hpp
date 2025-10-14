#pragma once

#include <app/window.hpp>

#include <game/physics.hpp>

#include <renderer/buffer.hpp>
#include <renderer/command_buffer.hpp>

#include <glm/glm.hpp>

namespace game {
    struct alignas(64) TileInstance {
        glm::vec3 position = {0.0f, 0.0f, 0.0f};
        glm::vec2 scale = {1.0f, 1.0f};
        glm::vec2 shear = {0.0f, 0.0f};

        float rotation = 0.0f;

        glm::vec2 textureLocation = {0.0f, 0.0f};
        glm::vec2 textureOffset = {0.0f, 0.0f};
        glm::vec2 textureScale = {1.0f, 1.0f};
    };

    struct TileMesh {
        renderer::Buffer vertexBuffer;
        renderer::Buffer instanceBuffer;
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
    void renderMap(TileMesh& mesh, Map& map, renderer::CommandBuffer& commandBuffer);
    void destroyMap(TileMesh& mesh);

    void loadMapFromFile(Map& map, const std::string& path);

    void resolveMapCollisions(const Map& map, std::span<MovableBody> bodies, std::span<BoxCollider> colliders, std::span<BoxCollisionResult> results);
}