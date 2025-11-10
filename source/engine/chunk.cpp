#include <engine/chunk.hpp>
#include <engine/engine.hpp>
#include <engine/tile_pool.hpp>

#include <components/tags.hpp>
#include <components/transforms.hpp>

#include <random>

#include <glm/gtc/noise.hpp>

glm::vec2 engine::worldToScreenSpace(glm::vec3 position) {
    constexpr float radX = 0.463647609001f;
    constexpr float radZ = M_PI - radX;

    glm::vec2 dirX = {std::cos(radX), std::sin(radX)};
    glm::vec2 dirY = {0.0f, 1.0f};
    glm::vec2 dirZ = {std::cos(radZ), std::sin(radZ)};

    constexpr glm::vec3 unitScale = {0.558f, 0.5f, 0.558f};

    return (dirX * position.x * unitScale.x + dirY * position.y * unitScale.y + dirZ * position.z * unitScale.z);
}

glm::vec3 engine::screenToWorldSpace(glm::vec2 screen, float y) {
    constexpr float radX = 0.463647609001f;
    constexpr float radZ = M_PI - radX;

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

    auto fbm2D = [](glm::vec2 p, int octaves = 4, float lacunarity = 2.0f, float gain = 0.5f) {
        float value = 0.0f;
        float amplitude = 1.0f;
        float frequency = 1.0f;

        for (int i = 0; i < octaves; ++i) {
            value += glm::simplex(p * frequency) * amplitude;
            frequency *= lacunarity;
            amplitude *= gain;
        }

        return value;
    };

    static std::random_device randomDevice;
    static std::mt19937 generator(randomDevice());

    auto randomRange = [](float min, float max) {
        std::uniform_real_distribution<float> distribution(min, max);

        return distribution(generator);
    };

    for (std::int64_t y = 0; y < chunkExtent.y; y++) {
        for (std::int64_t x = 0; x < chunkExtent.x; x++) {
            for (std::int64_t z = 0; z < chunkExtent.z; z++) {
                auto entity = registry.create();

                auto& proxy = registry.emplace<components::TileProxy>(entity, tilePool.insert({}, 0));
                auto& instance = tilePool.getInstance(proxy);
                auto& data = tilePool.getData(proxy);

                glm::ivec3 worldPosition = chunk.position + glm::ivec3{x, y, z};

                registry.emplace<components::Position>(entity, worldPosition);
                registry.emplace<components::TileTag>(entity);

                float noise = fbm2D(glm::vec2(worldPosition.x, worldPosition.z) * 0.005f, 12, 1.7, 0.6);

                noise = (noise + 1.0f) * 0.5f;

                float value;

                float rand = randomRange(0.9, 1.0);

                if (noise > 0.4f) {
                    value = 0.0f;
                    rand -= noise * 0.3f;
                }
                else if (noise > 0.3f) {
                    value = 0.4f;
                }
                else {
                    value = 0.5f;
                    rand += noise * 0.7f;
                }

                instance.transform.scale = {1.0f, 1.0f};
                instance.transform.position = worldToScreenSpace(worldPosition);

                instance.appearance.texture.offset = {0.0, 0.0};
                instance.appearance.texture.repeat = {1.0, 1.0};
                instance.appearance.texture.sample.extent = {0.1, 0.1};
                instance.appearance.texture.sample.position = {value, 0.0};
                instance.appearance.colourFactor = {rand, rand, rand, 1.0};

                chunk.tiles.push_back(entity);
            }
        }
    }

    auto view = registry.view<components::TileProxy, components::Position, components::TileTag>();

    auto worldSizeTiles = worldGenerator.getWorldSize() * worldGenerator.getChunkSize();

    for (auto [entity, proxy, position] : view.each()) {
        auto& data = tilePool.getData(proxy);

        data.order = (worldSizeTiles.y - position.position.y) * (worldSizeTiles.x + worldSizeTiles.z - 1) + position.position.x + position.position.z;
    }
}

void engine::unloadChunk(engine::Chunk& chunk, engine::Engine& engine) {
    auto& registry = engine.getRegistry();
    auto& tilePool = engine.getEntityTilePool();

    for (auto& tile : chunk.tiles) {
        auto& proxy = registry.get<components::TileProxy>(tile);

        tilePool.remove(proxy);
        registry.destroy(tile);
    }

    chunk.tiles.clear();
}