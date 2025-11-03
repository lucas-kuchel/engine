#include <engine/engine.hpp>
#include <engine/world_generator.hpp>

#include <components/transforms.hpp>

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
    auto camera = engine_.getCurrentCamera();

    auto& registry = engine_.getRegistry();
    auto& cameraScale = registry.get<components::Scale>(camera);
    auto& cameraPosition = registry.get<components::Position>(camera);

    glm::vec2 cameraScreenPos = engine::worldToScreenSpace(cameraPosition.position);
    glm::vec2 cameraChunkPos = glm::floor(cameraScreenPos * glm::vec2(1 / chunkSize_.x, 1 / chunkSize_.z));
    glm::vec2 halfScale = cameraScale.scale * 0.5f;
    glm::vec2 minVisibleArea = cameraScreenPos - halfScale;
    glm::vec2 maxVisibleArea = cameraScreenPos + halfScale;

    loadedChunks_.reserve(static_cast<std::size_t>(worldSize_.x * worldSize_.y * worldSize_.z));

    for (int y = 0; y < worldSize_.y; ++y) {
        for (int x = -worldSize_.x; x < worldSize_.x; ++x) {
            for (int z = -worldSize_.z; z < worldSize_.z; ++z) {
                glm::ivec3 chunkPosWorld = glm::ivec3(cameraChunkPos.x, 0.0, cameraChunkPos.y) + (chunkSize_ * glm::ivec3{x, y, z});
                glm::vec2 chunkScreenPos = engine::worldToScreenSpace(glm::vec3(chunkPosWorld));

                bool chunkExists = loadedChunks_.contains(chunkPosWorld);
                bool isInvalidPosition = chunkScreenPos.x + static_cast<float>(chunkSize_.x) < minVisibleArea.x ||
                                         chunkScreenPos.x - static_cast<float>(chunkSize_.x) > maxVisibleArea.x ||
                                         chunkScreenPos.y + static_cast<float>(chunkSize_.z) < minVisibleArea.y ||
                                         chunkScreenPos.y - static_cast<float>(chunkSize_.z) > maxVisibleArea.y;

                if (isInvalidPosition && chunkExists) {
                    auto& chunk = loadedChunks_[chunkPosWorld];

                    unloadChunk(chunk, engine_);

                    loadedChunks_.erase(chunkPosWorld);
                }
                else if (!isInvalidPosition && !chunkExists) {
                    auto& chunk = loadedChunks_[chunkPosWorld];
                    chunk.position = chunkPosWorld;

                    generateChunk(chunk, engine_);
                }
            }
        }
    }
}
