#pragma once

#include <vector>

#include <entt/entt.hpp>
#include <glm/glm.hpp>

namespace engine {
    class Engine;

    struct Chunk {
        std::vector<entt::entity> tiles;

        glm::ivec3 position;
    };

    struct ChunkOccupationMap {
        ChunkOccupationMap(glm::uvec3 dimensions, const glm::ivec3& pos) {
            entries.resize(dimensions.x, std::vector<std::vector<std::uint8_t>>(dimensions.y, std::vector<std::uint8_t>(dimensions.z, 0)));
            position = pos;
        }

        ChunkOccupationMap() = default;

        std::vector<std::vector<std::vector<std::uint8_t>>> entries;

        glm::ivec3 position;
    };

    glm::vec2 worldToScreenSpace(glm::vec3 position);
    glm::vec3 screenToWorldSpace(glm::vec2 screen, float y = 0.0f);

    void determineChunkTiles(engine::ChunkOccupationMap& occupationMap, engine::Engine& engine);
    void generateChunk(engine::Chunk& chunk, engine::ChunkOccupationMap& occupationMap, engine::Engine& engine);
    void sortTiles(engine::Engine& engine);
    void unloadChunk(engine::Chunk& chunk, engine::Engine& engine);
}