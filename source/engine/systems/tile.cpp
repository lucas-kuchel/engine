#include <engine/components/tile.hpp>
#include <engine/components/world.hpp>
#include <engine/engine.hpp>
#include <engine/systems/tile.hpp>

#include <cstring>

void engine::systems::createTileMeshes(entt::registry& registry, Engine& engine, renderer::Device& device, renderer::CommandBuffer& commandBuffer, renderer::Buffer& stagingBuffer, std::uint64_t& stagingBufferOffset) {
    if (!stagingBuffer) {
        return;
    }

    for (auto& entity : registry.view<components::TileMesh>()) {
        auto& tileMesh = registry.get<components::TileMesh>(entity);

        if (tileMesh.instanceBuffer || tileMesh.vertexBuffer || !renderer::CommandBuffer::capturing(commandBuffer)) {
            continue;
        }

        std::array<glm::vec2, 4> vertices = {
            glm::vec2{1.0, 1.0},
            glm::vec2{0.0, 1.0},
            glm::vec2{1.0, 0.0},
            glm::vec2{0.0, 0.0},
        };

        renderer::BufferCreateInfo vertexBufferCreateInfo = {
            .device = device,
            .memoryType = renderer::MemoryType::DEVICE_LOCAL,
            .usageFlags = renderer::BufferUsageFlags::VERTEX | renderer::BufferUsageFlags::TRANSFER_DESTINATION,
            .sizeBytes = sizeof(vertices),
        };

        renderer::BufferCreateInfo instanceBufferCreateInfo = {
            .device = device,
            .memoryType = renderer::MemoryType::DEVICE_LOCAL,
            .usageFlags = renderer::BufferUsageFlags::VERTEX | renderer::BufferUsageFlags::TRANSFER_DESTINATION,
            .sizeBytes = engine.getTiles().size() * sizeof(components::Tile),
        };

        tileMesh.vertexBuffer = renderer::Buffer::create(vertexBufferCreateInfo);
        tileMesh.instanceBuffer = renderer::Buffer::create(instanceBufferCreateInfo);

        std::size_t vertexBufferSize = renderer::Buffer::size(tileMesh.vertexBuffer);
        std::size_t instanceBufferSize = renderer::Buffer::size(tileMesh.instanceBuffer);
        std::size_t totalSize = vertexBufferSize + instanceBufferSize;

        auto mapping = renderer::Buffer::map(stagingBuffer, totalSize, stagingBufferOffset);

        std::memcpy(mapping.data.data(), vertices.data(), vertexBufferSize);
        std::memcpy(mapping.data.data() + vertexBufferSize, engine.getTiles().data(), instanceBufferSize);

        renderer::Buffer::unmap(stagingBuffer, mapping);

        renderer::BufferCopyRegion vertexCopyRegion = {
            .sourceOffsetBytes = stagingBufferOffset,
            .destinationOffsetBytes = 0,
            .sizeBytes = vertexBufferSize,
        };

        renderer::BufferCopyRegion instanceCopyRegion = {
            .sourceOffsetBytes = stagingBufferOffset + vertexCopyRegion.sizeBytes,
            .destinationOffsetBytes = 0,
            .sizeBytes = instanceBufferSize,
        };

        stagingBufferOffset += totalSize;

        renderer::CommandBuffer::copyBuffer(commandBuffer, stagingBuffer, tileMesh.vertexBuffer, {vertexCopyRegion});
        renderer::CommandBuffer::copyBuffer(commandBuffer, stagingBuffer, tileMesh.instanceBuffer, {instanceCopyRegion});
    }
}

void engine::systems::updateTileMeshes(entt::registry& registry, Engine& engine, renderer::CommandBuffer& commandBuffer, renderer::Buffer& stagingBuffer, std::uint64_t& stagingBufferOffset) {
    if (!stagingBuffer) {
        return;
    }

    for (auto& entity : registry.view<components::TileMesh>()) {
        auto& tileMesh = registry.get<components::TileMesh>(entity);

        if (!tileMesh.instanceBuffer || !tileMesh.vertexBuffer || !renderer::CommandBuffer::capturing(commandBuffer)) {
            continue;
        }

        std::size_t instanceBufferSize = renderer::Buffer::size(tileMesh.instanceBuffer);

        auto mapping = renderer::Buffer::map(stagingBuffer, instanceBufferSize, stagingBufferOffset);

        std::memcpy(mapping.data.data(), engine.getTiles().data(), instanceBufferSize);

        renderer::Buffer::unmap(stagingBuffer, mapping);

        renderer::BufferCopyRegion instanceCopyRegion = {
            .sourceOffsetBytes = stagingBufferOffset,
            .destinationOffsetBytes = 0,
            .sizeBytes = instanceBufferSize,
        };

        stagingBufferOffset += instanceBufferSize;

        renderer::CommandBuffer::copyBuffer(commandBuffer, stagingBuffer, tileMesh.instanceBuffer, {instanceCopyRegion});
    }
}

void engine::systems::destroyTileMeshes(entt::registry& registry) {
    for (auto& entity : registry.view<components::TileMesh>()) {
        auto& tileMesh = registry.get<components::TileMesh>(entity);

        if (tileMesh.instanceBuffer) {
            renderer::Buffer::destroy(tileMesh.instanceBuffer);
        }

        if (tileMesh.vertexBuffer) {
            renderer::Buffer::destroy(tileMesh.vertexBuffer);
        }
    }
}