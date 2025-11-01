#include <engine/engine.hpp>
#include <engine/tile_mesh.hpp>

#include <cstring>

engine::TileMesh::TileMesh(Engine& engine)
    : engine_(engine) {
    auto& renderer = engine.getRenderer();
    auto& device = renderer.getDevice();

    renderer::BufferCreateInfo createInfo = {
        .device = device,
        .memoryType = renderer::MemoryType::DEVICE_LOCAL,
        .usageFlags = renderer::BufferUsageFlags::VERTEX | renderer::BufferUsageFlags::TRANSFER_DESTINATION,
        .sizeBytes = sizeof(std::array<glm::vec2, 4>),
    };

    meshBuffer_ = renderer::Buffer::create(createInfo);
}

engine::TileMesh::~TileMesh() {
    if (instanceBuffer_) {
        renderer::Buffer::destroy(instanceBuffer_);
    }

    if (meshBuffer_) {
        renderer::Buffer::destroy(meshBuffer_);
    }
}

void engine::TileMesh::setBaseMesh(const std::array<glm::vec2, 4>& vertices) {
    auto& stagingManager = engine_.getStagingManager();
    auto& transferBuffer = engine_.getTransferBuffer();
    auto& stagingBuffer = stagingManager.getCurrentBuffer();
    auto& stagingOffset = stagingManager.getOffset();

    auto mapping = renderer::Buffer::map(stagingBuffer, sizeof(vertices), stagingOffset);

    std::memcpy(mapping.data.data(), vertices.data(), sizeof(vertices));

    renderer::Buffer::unmap(stagingBuffer, mapping);

    renderer::BufferCopyRegion copyRegion = {
        .sourceOffsetBytes = stagingOffset,
        .destinationOffsetBytes = 0,
        .sizeBytes = sizeof(vertices),
    };

    renderer::CommandBuffer::copyBuffer(transferBuffer, stagingBuffer, meshBuffer_, {copyRegion});

    stagingOffset += sizeof(vertices);
}

void engine::TileMesh::createInstanceBuffer(std::size_t instanceCount) {
    if (instanceBuffer_) {
        renderer::Buffer::destroy(instanceBuffer_);
    }

    auto& renderer = engine_.getRenderer();
    auto& device = renderer.getDevice();

    renderer::BufferCreateInfo createInfo = {
        .device = device,
        .memoryType = renderer::MemoryType::DEVICE_LOCAL,
        .usageFlags = renderer::BufferUsageFlags::VERTEX | renderer::BufferUsageFlags::TRANSFER_DESTINATION,
        .sizeBytes = instanceCount * sizeof(TileInstance),
    };

    instanceBuffer_ = renderer::Buffer::create(createInfo);
}

void engine::TileMesh::setInstances(std::span<TileInstance> instances) {
    auto& stagingManager = engine_.getStagingManager();
    auto& transferBuffer = engine_.getTransferBuffer();
    auto& stagingBuffer = stagingManager.getCurrentBuffer();
    auto& stagingOffset = stagingManager.getOffset();

    auto mapping = renderer::Buffer::map(stagingBuffer, sizeof(TileInstance) * instances.size(), stagingOffset);

    std::memcpy(mapping.data.data(), instances.data(), sizeof(TileInstance) * instances.size());

    renderer::Buffer::unmap(stagingBuffer, mapping);

    renderer::BufferCopyRegion copyRegion = {
        .sourceOffsetBytes = stagingOffset,
        .destinationOffsetBytes = 0,
        .sizeBytes = sizeof(TileInstance) * instances.size(),
    };

    renderer::CommandBuffer::copyBuffer(transferBuffer, stagingBuffer, instanceBuffer_, {copyRegion});

    stagingOffset += sizeof(TileInstance) * instances.size();
}