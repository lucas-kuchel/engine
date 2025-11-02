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
    glm::vec2 halfScale = cameraScale.scale * 0.5f;

    loadedChunks_.reserve(static_cast<std::size_t>(worldSize_.x * worldSize_.y * worldSize_.z));

    for (int y = 0; y < worldSize_.y; ++y) {
        for (int x = -worldSize_.x; x < worldSize_.x; ++x) {
            for (int z = -worldSize_.z; z < worldSize_.z; ++z) {
                glm::ivec3 chunkPosWorld = chunkSize_ * glm::ivec3{x, y, z};
                glm::vec2 chunkScreenPos = engine::worldToScreenSpace(glm::vec3(chunkPosWorld));
                glm::vec2 minVisible = cameraScreenPos - halfScale;
                glm::vec2 maxVisible = cameraScreenPos + halfScale;

                if (chunkScreenPos.x + static_cast<float>(chunkSize_.x) < minVisible.x ||
                    chunkScreenPos.x - static_cast<float>(chunkSize_.x) > maxVisible.x ||
                    chunkScreenPos.y + static_cast<float>(chunkSize_.z) < minVisible.y ||
                    chunkScreenPos.y - static_cast<float>(chunkSize_.z) > maxVisible.y) {
                    continue;
                }

                auto& chunk = loadedChunks_.emplace_back();
                chunk.position = chunkPosWorld;

                generateChunk(chunk, engine_);
                sortChunk(chunk, engine_);
            }
        }
    }
}
