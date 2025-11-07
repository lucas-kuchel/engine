#include <engine/engine.hpp>
#include <engine/tile_mesh.hpp>

#include <cstring>

void engine::TileMesh::create(Engine& engine) {
    engine_ = &engine;

    auto& renderer = engine_->getRenderer();
    auto& device = renderer.getDevice();

    vulkanite::renderer::BufferCreateInfo bufferCreateInfo = {
        .device = device,
        .memoryType = vulkanite::renderer::MemoryType::DEVICE_LOCAL,
        .usageFlags = vulkanite::renderer::BufferUsageFlags::VERTEX | vulkanite::renderer::BufferUsageFlags::TRANSFER_DESTINATION,
        .sizeBytes = sizeof(std::array<glm::vec2, 4>),
    };

    meshBuffer_.create(bufferCreateInfo);
}

engine::TileMesh::~TileMesh() {
    if (meshBuffer_) {
        meshBuffer_.destroy();
    }

    if (instanceBuffer_) {
        instanceBuffer_.destroy();
    }
}

void engine::TileMesh::setBaseMesh(const std::array<glm::vec2, 4>& vertices) {
    auto& stagingManager = engine_->getStagingManager();
    auto& transferBuffer = engine_->getTransferBuffer();
    auto& stagingBuffer = stagingManager.getCurrentBuffer();
    auto& stagingOffset = stagingManager.getOffset();

    auto mapping = stagingBuffer.map(sizeof(vertices), stagingOffset);

    std::memcpy(mapping.data.data(), vertices.data(), sizeof(vertices));

    stagingBuffer.unmap(mapping);

    vulkanite::renderer::BufferCopyRegion copyRegion = {
        .sourceOffsetBytes = stagingOffset,
        .destinationOffsetBytes = 0,
        .sizeBytes = sizeof(vertices),
    };

    transferBuffer.copyBuffer(stagingBuffer, meshBuffer_, {copyRegion});

    stagingOffset += sizeof(vertices);
}

void engine::TileMesh::createInstanceBuffer(std::size_t instanceCount) {
    if (instanceBuffer_) {
        instanceBuffer_.destroy();
    }

    auto& renderer = engine_->getRenderer();
    auto& device = renderer.getDevice();

    vulkanite::renderer::BufferCreateInfo createInfo = {
        .device = device,
        .memoryType = vulkanite::renderer::MemoryType::DEVICE_LOCAL,
        .usageFlags = vulkanite::renderer::BufferUsageFlags::VERTEX | vulkanite::renderer::BufferUsageFlags::TRANSFER_DESTINATION,
        .sizeBytes = instanceCount * sizeof(TileInstance),
    };

    instanceBuffer_.create(createInfo);
}

void engine::TileMesh::setInstances(std::span<TileInstance> instances) {
    auto& stagingManager = engine_->getStagingManager();
    auto& transferBuffer = engine_->getTransferBuffer();
    auto& stagingBuffer = stagingManager.getCurrentBuffer();
    auto& stagingOffset = stagingManager.getOffset();

    auto mapping = stagingBuffer.map(sizeof(TileInstance) * instances.size(), stagingOffset);

    std::memcpy(mapping.data.data(), instances.data(), sizeof(TileInstance) * instances.size());

    stagingBuffer.unmap(mapping);

    vulkanite::renderer::BufferCopyRegion copyRegion = {
        .sourceOffsetBytes = stagingOffset,
        .destinationOffsetBytes = 0,
        .sizeBytes = sizeof(TileInstance) * instances.size(),
    };

    transferBuffer.copyBuffer(stagingBuffer, instanceBuffer_, {copyRegion});

    stagingOffset += sizeof(TileInstance) * instances.size();
}