#include <engine/world_generator.hpp>

engine::WorldGenerator::WorldGenerator(Engine& engine)
    : engine_(engine) {
}

void engine::WorldGenerator::setWorldSize(glm::ivec3 size) {
    worldSize_ = size;
}

void engine::WorldGenerator::setChunkSize(glm::ivec3 size) {
    chunkSize_ = size;
}

void engine::WorldGenerator::generate() {
    loadedChunks_.reserve(static_cast<std::size_t>(worldSize_.x * worldSize_.y * worldSize_.z));

    for (std::int64_t y = -worldSize_.y; y < worldSize_.y; y++) {
        for (std::int64_t x = -worldSize_.x; x < worldSize_.x; x++) {
            for (std::int64_t z = -worldSize_.z; z < worldSize_.z; z++) {
                auto chunkPosition = chunkSize_ * glm::ivec3{x, y, z};

                auto& chunk = loadedChunks_.emplace_back();

                chunk.position = chunkPosition;

                generateChunk(chunk, engine_);
                sortChunk(chunk, engine_);
            }
        }
    }
}