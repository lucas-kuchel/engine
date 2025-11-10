#pragma once

#include <engine/chunk.hpp>

#include <unordered_map>

namespace std {
    template <>
    struct hash<glm::ivec3> {
        std::size_t operator()(const glm::ivec3& v) const noexcept {
            std::size_t h1 = std::hash<int>()(v.x);
            std::size_t h2 = std::hash<int>()(v.y);
            std::size_t h3 = std::hash<int>()(v.z);

            std::size_t seed = h1;
            seed ^= h2 + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            seed ^= h3 + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            return seed;
        }
    };
}

namespace engine {
    class Engine;

    class WorldNoiseSystem {
    public:
        float sample(const glm::vec3& position);

    private:
    };

    struct Tile {
        glm::vec2 textureOffset = {0.0, 0.0};
        glm::vec2 textureScale = {0.1, 0.1};

        bool visible = false;
    };

    class WorldGenerator {
    public:
        WorldGenerator(Engine& engine);

        void setWorldSize(glm::ivec3 size);
        void setChunkSize(glm::ivec3 size);

        glm::ivec3 getWorldSize() const {
            return worldSize_;
        }

        glm::ivec3 getChunkSize() const {
            return chunkSize_;
        }

        std::span<Tile> getAvailableTiles() {
            return availableTiles_;
        }

        void generate();

    private:
        std::unordered_map<glm::ivec3, Chunk> loadedChunks_;
        std::unordered_map<glm::ivec3, ChunkOccupationMap> loadedChunkOccupations_;

        std::array<Tile, 256> availableTiles_;

        Engine& engine_;

        glm::ivec3 worldSize_;
        glm::uvec3 chunkSize_;
    };
}