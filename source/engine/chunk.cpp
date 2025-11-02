#include <engine/engine.hpp>
#include <engine/tile_pool.hpp>

#include <engine/chunk.hpp>

glm::vec2 engine::worldToScreenSpace(glm::vec3 position) {
    constexpr float radX = 0.463647609001f;
    constexpr float radZ = M_PIf - radX;

    glm::vec2 dirX = {std::cos(radX), std::sin(radX)};
    glm::vec2 dirY = {0.0f, 1.0f};
    glm::vec2 dirZ = {std::cos(radZ), std::sin(radZ)};

    constexpr glm::vec3 unitScale = {0.558f, 0.5f, 0.558f};

    return (dirX * position.x * unitScale.x + dirY * position.y * unitScale.y + dirZ * position.z * unitScale.z);
}

glm::vec3 engine::screenToWorldSpace(glm::vec2 screen, float y) {
    constexpr float radX = 0.463647609001f;
    constexpr float radZ = M_PIf - radX;

    constexpr glm::vec3 unitScale = {0.558f, 0.5f, 0.558f};

    const float u = screen.x;
    const float v = screen.y - y * unitScale.y;

    const float a = std::cos(radX) * unitScale.x;
    const float b = std::cos(radZ) * unitScale.z;
    const float c = std::sin(radX) * unitScale.x;
    const float d = std::sin(radZ) * unitScale.z;

    const float determinant = a * d - b * c;

    const float x = (u * d - b * v) / determinant;
    const float z = (-u * c + a * v) / determinant;

    return {x, y, z};
}

void engine::generateChunk(engine::Chunk& chunk, engine::Engine& engine) {
    auto& registry = engine.getRegistry();
    auto& tilePool = engine.getEntityTilePool();
    auto& worldGenerator = engine.getWorldGenerator();

    auto chunkExtent = worldGenerator.getChunkSize();

    chunk.tiles.reserve(static_cast<std::size_t>(chunkExtent.x * chunkExtent.y * chunkExtent.z));

    for (std::int64_t y = 0; y < chunkExtent.y; y++) {
        for (std::int64_t x = 0; x < chunkExtent.x; x++) {
            for (std::int64_t z = 0; z < chunkExtent.z; z++) {
                auto entity = registry.create();

                auto& proxy = registry.emplace<components::TileProxy>(entity, tilePool.insert({}, 0));
                auto& instance = tilePool.getInstance(proxy);
                auto& data = tilePool.getData(proxy);

                glm::ivec3 worldPosition = chunk.position + glm::ivec3{x, y, z};

                data.order = (chunkExtent.y - worldPosition.y) * (chunkExtent.x + chunkExtent.z - 1) + worldPosition.x + worldPosition.z;

                instance.transform.scale = {1.0f, 1.0f};
                instance.transform.position = worldToScreenSpace(worldPosition);

                instance.appearance.texture.offset = {0.0, 0.0};
                instance.appearance.texture.repeat = {1.0, 1.0};
                instance.appearance.texture.sample.extent = {0.1, 0.1};
                instance.appearance.texture.sample.position = {0.3, 0.0};
                instance.appearance.colourFactor = {1.0, 1.0, 1.0, 1.0};

                chunk.tiles.push_back(entity);
            }
        }
    }
}

void engine::sortChunk(engine::Chunk&, engine::Engine&) {
}