#include <game/map.hpp>

#include <cstring>
#include <fstream>

#include <nlohmann/json.hpp>

namespace game {
    void createMap(TileMesh& mesh, Map& map, renderer::Device& device, renderer::Buffer& stagingBuffer, std::uint64_t& stagingBufferOffset, renderer::CommandBuffer& transferBuffer) {
        std::array<TileVertex, 4> vertices = {
            TileVertex({0.5, -0.5}, {1.0, 0.0}),
            TileVertex({-0.5, -0.5}, {0.0, 0.0}),
            TileVertex({0.5, 0.5}, {1.0, 1.0}),
            TileVertex({-0.5, 0.5}, {0.0, 1.0}),
        };

        renderer::BufferCreateInfo vertexBufferCreateInfo = {
            .device = device,
            .memoryType = renderer::MemoryType::DEVICE_LOCAL,
            .usageFlags = renderer::BufferUsageFlags::VERTEX | renderer::BufferUsageFlags::TRANSFER_DESTINATION,
            .sizeBytes = vertices.size() * sizeof(TileVertex),
        };

        renderer::BufferCreateInfo instanceBufferCreateInfo = {
            .device = device,
            .memoryType = renderer::MemoryType::DEVICE_LOCAL,
            .usageFlags = renderer::BufferUsageFlags::VERTEX | renderer::BufferUsageFlags::TRANSFER_DESTINATION,
            .sizeBytes = map.tiles.size() * sizeof(TileInstance),
        };

        mesh.vertexBuffer = data::makeUnique<renderer::Buffer>(vertexBufferCreateInfo);
        mesh.instanceBuffer = data::makeUnique<renderer::Buffer>(instanceBufferCreateInfo);

        std::uint64_t totalSize = mesh.vertexBuffer->size() + mesh.instanceBuffer->size();
        std::span<std::uint8_t> stagingBufferData = stagingBuffer.map(totalSize, stagingBufferOffset);

        std::memcpy(stagingBufferData.data(), vertices.data(), mesh.vertexBuffer->size());
        std::memcpy(stagingBufferData.data() + mesh.vertexBuffer->size(), map.tiles.data(), mesh.instanceBuffer->size());

        stagingBuffer.unmap();

        renderer::BufferCopyRegion vertexBufferCopyRegion = {
            .sourceOffsetBytes = stagingBufferOffset,
            .destinationOffsetBytes = 0,
            .sizeBytes = mesh.vertexBuffer->size(),
        };

        renderer::BufferCopyRegion instanceBufferCopyRegion = {
            .sourceOffsetBytes = stagingBufferOffset + mesh.vertexBuffer->size(),
            .destinationOffsetBytes = 0,
            .sizeBytes = mesh.instanceBuffer->size(),
        };

        stagingBufferOffset += mesh.vertexBuffer->size() + mesh.instanceBuffer->size();

        transferBuffer.copyBuffer(stagingBuffer, mesh.vertexBuffer.ref(), {vertexBufferCopyRegion});
        transferBuffer.copyBuffer(stagingBuffer, mesh.instanceBuffer.ref(), {instanceBufferCopyRegion});
    }

    void updateMap(TileMesh& mesh, Map& map, renderer::Buffer& stagingBuffer, std::uint64_t& stagingBufferOffset, renderer::CommandBuffer& transferBuffer) {
        std::span<std::uint8_t> stagingBufferData = stagingBuffer.map(mesh.instanceBuffer->size(), stagingBufferOffset);

        std::memcpy(stagingBufferData.data(), map.tiles.data(), mesh.instanceBuffer->size());

        stagingBuffer.unmap();

        renderer::BufferCopyRegion instanceBufferCopyRegion = {
            .sourceOffsetBytes = stagingBufferOffset,
            .destinationOffsetBytes = 0,
            .sizeBytes = mesh.instanceBuffer->size(),
        };

        stagingBufferOffset += mesh.instanceBuffer->size();

        transferBuffer.copyBuffer(stagingBuffer, mesh.instanceBuffer.ref(), {instanceBufferCopyRegion});
    }

    void renderMap(TileMesh& mesh, Map& map, renderer::CommandBuffer& commandBuffer) {
        commandBuffer.bindVertexBuffers({mesh.vertexBuffer.ref(), mesh.instanceBuffer.ref()}, {0, 0}, 0);
        commandBuffer.draw(4, static_cast<std::uint32_t>(map.tiles.size()), 0, 0);
    }

    void loadMapFromFile(Map& map, const std::string& path) {
        std::ifstream file(path);

        if (!file) {
            throw std::runtime_error(std::format("Call failed: game::loadMapFromFile(): Failed to open file: {}", path));
        }

        std::string contents(std::istreambuf_iterator<char>(file), {});
        nlohmann::json json = nlohmann::json::parse(contents);

        if (json.contains("physics") && json["physics"].is_object()) {
            const auto& physicsJson = json["physics"];

            map.physics.gravity = physicsJson.value("gravity", 0.0f);
            map.physics.airResistance = physicsJson.value("airResistance", 0.0f);
        }

        if (json.contains("tiles") && json["tiles"].is_array()) {
            for (const auto& gridJson : json["tiles"]) {

                glm::uvec2 dimensions{0, 0};

                if (gridJson.contains("dimensions")) {
                    auto g = gridJson["dimensions"];
                    dimensions.x = g.size() > 0 ? g[0].get<std::uint32_t>() : 0;
                    dimensions.y = g.size() > 1 ? g[1].get<std::uint32_t>() : 0;
                }

                if (dimensions.x == 0 || dimensions.y == 0)
                    continue;

                glm::vec2 offset{0.0f};
                if (gridJson.contains("offset")) {
                    auto o = gridJson["offset"];
                    offset.x = o.size() > 0 ? o[0].get<float>() : 0.0f;
                    offset.y = o.size() > 1 ? o[1].get<float>() : 0.0f;
                }

                TileInstance baseTile;

                if (gridJson.contains("position")) {
                    auto p = gridJson["position"];
                    baseTile.position.x = p.size() > 0 ? p[0].get<float>() : 0.0f;
                    baseTile.position.y = p.size() > 1 ? p[1].get<float>() : 0.0f;
                }

                if (gridJson.contains("scale")) {
                    auto s = gridJson["scale"];
                    baseTile.scale.x = s.size() > 0 ? s[0].get<float>() : 1.0f;
                    baseTile.scale.y = s.size() > 1 ? s[1].get<float>() : 1.0f;
                }

                if (gridJson.contains("texOffset")) {
                    auto t = gridJson["texOffset"];
                    baseTile.texOffset.x = t.size() > 0 ? t[0].get<float>() : 0.0f;
                    baseTile.texOffset.y = t.size() > 1 ? t[1].get<float>() : 0.0f;
                }

                for (std::size_t i = 0; i < dimensions.x; i++) {
                    for (std::size_t j = 0; j < dimensions.y; j++) {
                        TileInstance tile = baseTile;

                        tile.position.x += offset.x * static_cast<float>(i);
                        tile.position.y += offset.y * static_cast<float>(j);

                        map.tiles.push_back(tile);
                    }
                }
            }
        }

        if (json.contains("colliders") && json["colliders"].is_array()) {
            for (const auto& collJson : json["colliders"]) {
                BoxCollider collider;

                if (collJson.contains("physics") && collJson["physics"].is_object()) {
                    collider.physics.friction = collJson["physics"].value("friction", 1.0f);
                }

                if (collJson.contains("position")) {
                    auto p = collJson["position"];
                    collider.position.x = p.size() > 0 ? p[0].get<float>() : 0.0f;
                    collider.position.y = p.size() > 1 ? p[1].get<float>() : 0.0f;
                }

                if (collJson.contains("extent")) {
                    auto e = collJson["extent"];
                    collider.extent.x = e.size() > 0 ? e[0].get<float>() : 1.0f;
                    collider.extent.y = e.size() > 1 ? e[1].get<float>() : 1.0f;
                }

                collider.position += (collider.extent - 1.0f) * 0.5f;

                map.colliders.push_back(collider);
            }
        }
    }

    void resolveMapCollisions(const Map& map, std::span<MovableBody> bodies, std::span<BoxCollider> colliders, std::span<BoxCollisionResult> results) {
        for (std::size_t i = 0; i < bodies.size(); i++) {
            MovableBody& body = bodies[i];
            BoxCollider& box = colliders[i];
            BoxCollisionResult& resultOut = results[i];

            resultOut = {};

            for (const BoxCollider& mapBox : map.colliders) {
                BoxCollisionResult result;

                game::testCollisionAABB(box, mapBox, result);

                if (!result.collided) {
                    continue;
                }

                box.position += result.normal * result.penetration;
                body.position = box.position;

                if (result.penetration.x > 0.0f) {
                    body.velocity.x = 0.0f;
                }
                if (result.penetration.y > 0.0f) {
                    body.velocity.y = 0.0f;
                }

                resultOut = result;
            }
        }
    }
}