#include <game/map.hpp>

#include <cstring>
#include <fstream>

#include <nlohmann/json.hpp>

namespace game {
    void createMap(TileMesh& mesh, Map& map, renderer::Device& device, renderer::Buffer& stagingBuffer, std::uint64_t& stagingBufferOffset, renderer::CommandBuffer& transferBuffer) {
        std::array<TileVertex, 4> vertices = {
            TileVertex({0.5, -0.5}, {0.5, 0.0}),
            TileVertex({-0.5, -0.5}, {0.0, 0.0}),
            TileVertex({0.5, 0.5}, {0.5, 0.5}),
            TileVertex({-0.5, 0.5}, {0.0, 0.5}),
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

        mesh.vertexBuffer = renderer::Buffer::create(vertexBufferCreateInfo);
        mesh.instanceBuffer = renderer::Buffer::create(instanceBufferCreateInfo);

        std::uint64_t totalSize = renderer::Buffer::size(mesh.vertexBuffer) + renderer::Buffer::size(mesh.instanceBuffer);
        auto mapping = renderer::Buffer::map(stagingBuffer, totalSize, stagingBufferOffset);

        std::memcpy(mapping.data.data(), vertices.data(), renderer::Buffer::size(mesh.vertexBuffer));
        std::memcpy(mapping.data.data() + renderer::Buffer::size(mesh.vertexBuffer), map.tiles.data(), renderer::Buffer::size(mesh.instanceBuffer));

        renderer::Buffer::unmap(stagingBuffer, mapping);

        renderer::BufferCopyRegion vertexBufferCopyRegion = {
            .sourceOffsetBytes = stagingBufferOffset,
            .destinationOffsetBytes = 0,
            .sizeBytes = renderer::Buffer::size(mesh.vertexBuffer),
        };

        renderer::BufferCopyRegion instanceBufferCopyRegion = {
            .sourceOffsetBytes = stagingBufferOffset + renderer::Buffer::size(mesh.vertexBuffer),
            .destinationOffsetBytes = 0,
            .sizeBytes = renderer::Buffer::size(mesh.instanceBuffer),
        };

        stagingBufferOffset += totalSize;

        renderer::CommandBuffer::copyBuffer(transferBuffer, stagingBuffer, mesh.vertexBuffer, {vertexBufferCopyRegion});
        renderer::CommandBuffer::copyBuffer(transferBuffer, stagingBuffer, mesh.instanceBuffer, {instanceBufferCopyRegion});
    }

    void renderMap(TileMesh& mesh, Map& map, renderer::CommandBuffer& commandBuffer) {
        renderer::CommandBuffer::bindVertexBuffers(commandBuffer, {mesh.vertexBuffer, mesh.instanceBuffer}, {0, 0}, 0);
        renderer::CommandBuffer::draw(commandBuffer, 4, static_cast<std::uint32_t>(map.tiles.size()), 0, 0);
    }

    void destroyMap(TileMesh& mesh) {
        if (mesh.instanceBuffer) {
            renderer::Buffer::destroy(mesh.instanceBuffer);
        }

        if (mesh.vertexBuffer) {
            renderer::Buffer::destroy(mesh.vertexBuffer);
        }
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
            for (const auto& tileJson : json["tiles"]) {
                TileInstance instance;

                if (tileJson.contains("position")) {
                    auto p = tileJson["position"];
                    instance.position.x = p.size() > 0 ? p[0].get<float>() : 0.0f;
                    instance.position.y = p.size() > 1 ? p[1].get<float>() : 0.0f;
                    instance.position.z = p.size() > 2 ? p[2].get<float>() : 0.0f;
                }

                if (tileJson.contains("scale")) {
                    auto s = tileJson["scale"];
                    instance.scale.x = s.size() > 0 ? s[0].get<float>() : 1.0f;
                    instance.scale.y = s.size() > 1 ? s[1].get<float>() : 1.0f;
                }

                if (tileJson.contains("texOffset")) {
                    auto t = tileJson["texOffset"];
                    instance.texOffset.x = t.size() > 0 ? t[0].get<float>() : 0.0f;
                    instance.texOffset.y = t.size() > 1 ? t[1].get<float>() : 0.0f;
                }

                map.tiles.push_back(instance);
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

                if (collJson.contains("scale")) {
                    auto s = collJson["scale"];
                    collider.scale.x = s.size() > 0 ? s[0].get<float>() : 1.0f;
                    collider.scale.y = s.size() > 1 ? s[1].get<float>() : 1.0f;
                }

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