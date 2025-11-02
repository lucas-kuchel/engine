#pragma once

#include <engine/chunk.hpp>

namespace engine {
    class Engine;

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

        void generate();

    private:
        std::vector<Chunk> loadedChunks_;

        Engine& engine_;

        glm::ivec3 worldSize_;
        glm::ivec3 chunkSize_;
    };
}