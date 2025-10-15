#include <game/map.hpp>

#include <cstring>
#include <fstream>

#include <nlohmann/json.hpp>

#include <glm/gtc/matrix_transform.hpp>

namespace game {
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

    void loadMapFromFile(Map& map, const std::string& path) {
        std::ifstream file(path);

        if (!file) {
            throw std::runtime_error(std::format("Call failed: game::loadMapFromFile(): Failed to open file: {}", path));
        }

        std::string contents(std::istreambuf_iterator<char>(file), {});
        nlohmann::json json = nlohmann::json::parse(contents);

        if (json.contains("tiles") && json["tiles"].is_array()) {
            for (const auto& tileJson : json["tiles"]) {
                TileInstance instance;
                glm::mat4 model = {1.0f};
                glm::vec3 position = {0.0f, 0.0f, 0.0f};
                glm::vec3 scale = {1.0f, 1.0f, 1.0f};
                glm::vec3 shear = {0.0f, 0.0f, 0.0f};
                glm::vec3 rotation = {0.0f, 0.0f, 0.0f};

                if (tileJson.contains("transform") && tileJson["transform"].is_object()) {
                    const auto& t = tileJson["transform"];

                    if (t.contains("position")) {
                        auto p = t["position"];

                        position.x = p.size() > 0 ? p[0].get<float>() : 0.0f;
                        position.y = p.size() > 1 ? p[1].get<float>() : 0.0f;
                        position.z = p.size() > 2 ? p[2].get<float>() : 0.0f;
                    }

                    if (t.contains("scale")) {
                        auto s = t["scale"];

                        scale.x = s.size() > 0 ? s[0].get<float>() : 1.0f;
                        scale.y = s.size() > 1 ? s[1].get<float>() : 1.0f;
                        scale.z = s.size() > 2 ? s[2].get<float>() : 1.0f;
                    }

                    if (t.contains("shear")) {
                        auto sh = t["shear"];

                        shear.x = sh.size() > 0 ? sh[0].get<float>() : 0.0f;
                        shear.y = sh.size() > 1 ? sh[1].get<float>() : 0.0f;
                        shear.z = sh.size() > 2 ? sh[2].get<float>() : 0.0f;
                    }

                    if (t.contains("rotation")) {
                        auto r = t["rotation"];

                        rotation.x = r.size() > 0 ? r[0].get<float>() : 0.0f;
                        rotation.y = r.size() > 1 ? r[1].get<float>() : 0.0f;
                        rotation.z = r.size() > 2 ? r[2].get<float>() : 0.0f;
                    }
                }

                model = glm::scale(model, scale);

                if (glm::any(glm::notEqual(shear, glm::vec3(0.0f)))) {
                    glm::mat4 shearMatrix = {1.0f};

                    shearMatrix[1][0] = shear.x;
                    shearMatrix[2][0] = shear.y;
                    shearMatrix[2][1] = shear.z;

                    model *= shearMatrix;
                }

                model = glm::rotate(model, glm::radians(rotation.z), glm::vec3(0, 0, 1));
                model = glm::rotate(model, glm::radians(rotation.y), glm::vec3(0, 1, 0));
                model = glm::rotate(model, glm::radians(rotation.x), glm::vec3(1, 0, 0));

                model = glm::translate(glm::mat4(1.0f), position) * model;

                if (tileJson.contains("texture") && tileJson["texture"].is_object()) {
                    const auto& tex = tileJson["texture"];

                    if (tex.contains("position")) {
                        auto l = tex["position"];
                        instance.texturePosition.x = l.size() > 0 ? l[0].get<float>() : 0.0f;
                        instance.texturePosition.y = l.size() > 1 ? l[1].get<float>() : 0.0f;
                    }

                    if (tex.contains("extent")) {
                        auto l = tex["extent"];
                        instance.textureExtent.x = l.size() > 0 ? l[0].get<float>() : 0.0f;
                        instance.textureExtent.y = l.size() > 1 ? l[1].get<float>() : 0.0f;
                    }

                    if (tex.contains("offset")) {
                        auto o = tex["offset"];
                        instance.textureOffset.x = o.size() > 0 ? o[0].get<float>() : 0.0f;
                        instance.textureOffset.y = o.size() > 1 ? o[1].get<float>() : 0.0f;
                    }

                    if (tex.contains("scale")) {
                        auto s = tex["scale"];
                        instance.textureScale.x = s.size() > 0 ? s[0].get<float>() : 1.0f;
                        instance.textureScale.y = s.size() > 1 ? s[1].get<float>() : 1.0f;
                    }
                }

                map.instances.push_back(instance);
                map.models.push_back(model);
            }
        }
    }

    void resolveMapCollisions(const Map& map, std::span<MovableBody> bodies, std::span<Collider> colliders, std::span<CollisionResult> results) {
        for (std::size_t i = 0; i < bodies.size(); i++) {
            MovableBody& body = bodies[i];
            Collider& box = colliders[i];
            CollisionResult& resultOut = results[i];

            resultOut = {};

            for (const Collider& mapBox : map.colliders) {
                CollisionResult result;

                game::testCollisionOBB(box, mapBox, result);

                if (!result.collided) {
                    continue;
                }

                box.position += result.normal * result.penetration;
                body.position.x = box.position.x;
                body.position.z = box.position.y;

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