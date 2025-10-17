#include <game/map.hpp>
#include <game/mesh.hpp>
#include <game/tile.hpp>
#include <game/transforms.hpp>

#include <cstring>
#include <fstream>

#include <nlohmann/json.hpp>

#include <glm/gtc/matrix_transform.hpp>

namespace game {
    void loadMap(entt::registry& registry, const std::string& filepath) {
        std::ifstream mapFile(filepath);
        std::string mapFileContents(std::istreambuf_iterator<char>{mapFile}, {});
        nlohmann::json mapFileJson = nlohmann::json::parse(mapFileContents);

        if (mapFileJson.contains("tiles") && mapFileJson["tiles"].is_array()) {
            for (const auto& tileJson : mapFileJson["tiles"]) {
                auto tile = registry.create();
                auto tilePosition = registry.emplace<Position>(tile);
                auto tileScale = registry.emplace<Scale>(tile);
                auto tileShear = registry.emplace<Shear>(tile);
                auto tileRotation = registry.emplace<Rotation>(tile);
                auto tileTexture = registry.emplace<MeshTexture>(tile);

                registry.emplace<Transform>(tile);
                registry.emplace<StaticTileTag>(tile);

                if (tileJson.contains("transform") && tileJson["transform"].is_object()) {
                    const auto& tileEntry = tileJson["transform"];

                    if (tileEntry.contains("position")) {
                        auto position = tileEntry["position"];

                        tilePosition.position.x = position.size() > 0 ? position[0].get<float>() : 0.0f;
                        tilePosition.position.y = position.size() > 1 ? position[1].get<float>() : 0.0f;
                        tilePosition.position.z = position.size() > 2 ? position[2].get<float>() : 0.0f;
                    }

                    if (tileEntry.contains("scale")) {
                        auto scale = tileEntry["scale"];

                        tileScale.scale.x = scale.size() > 0 ? scale[0].get<float>() : 1.0f;
                        tileScale.scale.y = scale.size() > 1 ? scale[1].get<float>() : 1.0f;
                        tileScale.scale.z = scale.size() > 2 ? scale[2].get<float>() : 1.0f;
                    }

                    if (tileEntry.contains("shear")) {
                        auto shear = tileEntry["shear"];

                        tileShear.shear.x = shear.size() > 0 ? shear[0].get<float>() : 0.0f;
                        tileShear.shear.y = shear.size() > 1 ? shear[1].get<float>() : 0.0f;
                        tileShear.shear.z = shear.size() > 2 ? shear[2].get<float>() : 0.0f;
                    }

                    if (tileEntry.contains("rotation")) {
                        auto rotation = tileEntry["rotation"];

                        tileRotation.rotation.x = rotation.size() > 0 ? rotation[0].get<float>() : 0.0f;
                        tileRotation.rotation.y = rotation.size() > 1 ? rotation[1].get<float>() : 0.0f;
                        tileRotation.rotation.z = rotation.size() > 2 ? rotation[2].get<float>() : 0.0f;
                    }
                }

                if (tileJson.contains("texture") && tileJson["texture"].is_object()) {
                    const auto& texture = tileJson["texture"];

                    if (texture.contains("position")) {
                        auto position = texture["position"];

                        tileTexture.position.x = position.size() > 0 ? position[0].get<float>() : 0.0f;
                        tileTexture.position.y = position.size() > 1 ? position[1].get<float>() : 0.0f;
                    }

                    if (texture.contains("extent")) {
                        auto extent = texture["extent"];

                        tileTexture.extent.x = extent.size() > 0 ? extent[0].get<float>() : 0.0f;
                        tileTexture.extent.y = extent.size() > 1 ? extent[1].get<float>() : 0.0f;
                    }

                    if (texture.contains("offset")) {
                        auto offset = texture["offset"];

                        tileTexture.offset.x = offset.size() > 0 ? offset[0].get<float>() : 0.0f;
                        tileTexture.offset.y = offset.size() > 1 ? offset[1].get<float>() : 0.0f;
                    }

                    if (texture.contains("scale")) {
                        auto scale = texture["scale"];

                        tileTexture.scale.x = scale.size() > 0 ? scale[0].get<float>() : 1.0f;
                        tileTexture.scale.y = scale.size() > 1 ? scale[1].get<float>() : 1.0f;
                    }
                }
            }
        }
    }

    void createMap(Map& map, renderer::Device& device, renderer::Buffer& stagingBuffer, std::uint64_t& stagingBufferOffset, renderer::CommandBuffer& transferBuffer) {
        std::array<glm::vec3, 4> vertices = {
            glm::vec3{0.5, 0.5, 0.0},
            glm::vec3{-0.5, 0.5, 0.0},
            glm::vec3{0.5, -0.5, 0.0},
            glm::vec3{-0.5, -0.5, 0.0},
        };

        renderer::BufferCreateInfo vertexBufferCreateInfo = {
            .device = device,
            .memoryType = renderer::MemoryType::DEVICE_LOCAL,
            .usageFlags = renderer::BufferUsageFlags::VERTEX | renderer::BufferUsageFlags::TRANSFER_DESTINATION,
            .sizeBytes = vertices.size() * sizeof(glm::vec3),
        };

        renderer::BufferCreateInfo instanceBufferCreateInfo = {
            .device = device,
            .memoryType = renderer::MemoryType::DEVICE_LOCAL,
            .usageFlags = renderer::BufferUsageFlags::VERTEX | renderer::BufferUsageFlags::TRANSFER_DESTINATION,
            .sizeBytes = map.instances.size() * sizeof(TileInstance),
        };

        renderer::BufferCreateInfo modelBufferCreateInfo = {
            .device = device,
            .memoryType = renderer::MemoryType::DEVICE_LOCAL,
            .usageFlags = renderer::BufferUsageFlags::VERTEX | renderer::BufferUsageFlags::TRANSFER_DESTINATION,
            .sizeBytes = map.models.size() * sizeof(glm::mat4),
        };

        map.mesh.vertexBuffer = renderer::Buffer::create(vertexBufferCreateInfo);
        map.mesh.instanceBuffer = renderer::Buffer::create(instanceBufferCreateInfo);
        map.mesh.modelBuffer = renderer::Buffer::create(modelBufferCreateInfo);

        std::uint64_t teritarySize = renderer::Buffer::size(map.mesh.vertexBuffer);
        std::uint64_t secondarySize = teritarySize + renderer::Buffer::size(map.mesh.instanceBuffer);
        std::uint64_t totalSize = secondarySize + renderer::Buffer::size(map.mesh.modelBuffer);

        auto mapping = renderer::Buffer::map(stagingBuffer, totalSize, stagingBufferOffset);

        std::memcpy(mapping.data.data(), vertices.data(), renderer::Buffer::size(map.mesh.vertexBuffer));
        std::memcpy(mapping.data.data() + secondarySize, map.models.data(), renderer::Buffer::size(map.mesh.modelBuffer));
        std::memcpy(mapping.data.data() + teritarySize, map.instances.data(), renderer::Buffer::size(map.mesh.instanceBuffer));

        renderer::Buffer::unmap(stagingBuffer, mapping);

        renderer::BufferCopyRegion vertexBufferCopyRegion = {
            .sourceOffsetBytes = stagingBufferOffset,
            .destinationOffsetBytes = 0,
            .sizeBytes = renderer::Buffer::size(map.mesh.vertexBuffer),
        };

        renderer::BufferCopyRegion modelBufferCopyRegion = {
            .sourceOffsetBytes = stagingBufferOffset + secondarySize,
            .destinationOffsetBytes = 0,
            .sizeBytes = renderer::Buffer::size(map.mesh.modelBuffer),
        };

        renderer::BufferCopyRegion instanceBufferCopyRegion = {
            .sourceOffsetBytes = stagingBufferOffset + teritarySize,
            .destinationOffsetBytes = 0,
            .sizeBytes = renderer::Buffer::size(map.mesh.instanceBuffer),
        };

        stagingBufferOffset += totalSize;

        renderer::CommandBuffer::copyBuffer(transferBuffer, stagingBuffer, map.mesh.vertexBuffer, {vertexBufferCopyRegion});
        renderer::CommandBuffer::copyBuffer(transferBuffer, stagingBuffer, map.mesh.modelBuffer, {modelBufferCopyRegion});
        renderer::CommandBuffer::copyBuffer(transferBuffer, stagingBuffer, map.mesh.instanceBuffer, {instanceBufferCopyRegion});
    }

    void renderMap(Map& map, renderer::CommandBuffer& commandBuffer) {
        renderer::CommandBuffer::bindVertexBuffers(commandBuffer, {map.mesh.vertexBuffer, map.mesh.instanceBuffer, map.mesh.modelBuffer}, {0, 0, 0}, 0);
        renderer::CommandBuffer::draw(commandBuffer, 4, static_cast<std::uint32_t>(map.instances.size()), 0, 0);
    }

    void destroyMap(Map& map) {
        if (map.mesh.instanceBuffer) {
            renderer::Buffer::destroy(map.mesh.instanceBuffer);
        }

        if (map.mesh.vertexBuffer) {
            renderer::Buffer::destroy(map.mesh.vertexBuffer);
        }

        if (map.mesh.modelBuffer) {
            renderer::Buffer::destroy(map.mesh.modelBuffer);
        }
    }
}