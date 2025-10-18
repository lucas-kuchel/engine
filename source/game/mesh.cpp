#include <game/mesh.hpp>
#include <game/transforms.hpp>

#include <cstring>

namespace game {
    void createMesh(std::size_t tileCount, Mesh& mesh, renderer::Device& device, renderer::Buffer& stagingBuffer, renderer::CommandBuffer& commandBuffer, std::size_t stagingBufferOffset) {
        if (mesh.textureBuffer || mesh.transformBuffer || mesh.vertexBuffer || !stagingBuffer || tileCount == 0 || !renderer::CommandBuffer::capturing(commandBuffer)) {
            return;
        }

        std::array<MeshVertex, 4> vertices = {
            MeshVertex{{1.0, 1.0}},
            MeshVertex{{0.0, 1.0}},
            MeshVertex{{1.0, 0.0}},
            MeshVertex{{0.0, 0.0}},
        };

        renderer::BufferCreateInfo vertexBufferCreateInfo = {
            .device = device,
            .memoryType = renderer::MemoryType::DEVICE_LOCAL,
            .usageFlags = renderer::BufferUsageFlags::VERTEX | renderer::BufferUsageFlags::TRANSFER_DESTINATION,
            .sizeBytes = sizeof(vertices),
        };

        renderer::BufferCreateInfo textureBufferCreateInfo = {
            .device = device,
            .memoryType = renderer::MemoryType::DEVICE_LOCAL,
            .usageFlags = renderer::BufferUsageFlags::VERTEX | renderer::BufferUsageFlags::TRANSFER_DESTINATION,
            .sizeBytes = tileCount * sizeof(MeshTexture),
        };

        renderer::BufferCreateInfo transformBufferCreateInfo = {
            .device = device,
            .memoryType = renderer::MemoryType::DEVICE_LOCAL,
            .usageFlags = renderer::BufferUsageFlags::VERTEX | renderer::BufferUsageFlags::TRANSFER_DESTINATION,
            .sizeBytes = tileCount * sizeof(MeshTransform),
        };

        mesh.vertexBuffer = renderer::Buffer::create(vertexBufferCreateInfo);
        mesh.textureBuffer = renderer::Buffer::create(textureBufferCreateInfo);
        mesh.transformBuffer = renderer::Buffer::create(transformBufferCreateInfo);

        auto mapping = renderer::Buffer::map(stagingBuffer, renderer::Buffer::size(mesh.vertexBuffer), stagingBufferOffset);

        std::memcpy(mapping.data.data(), vertices.data(), sizeof(vertices));

        renderer::Buffer::unmap(stagingBuffer, mapping);

        renderer::BufferCopyRegion bufferCopyRegion = {
            .sourceOffsetBytes = stagingBufferOffset,
            .destinationOffsetBytes = 0,
            .sizeBytes = sizeof(vertices),
        };

        stagingBufferOffset += bufferCopyRegion.sizeBytes;

        renderer::CommandBuffer::copyBuffer(commandBuffer, stagingBuffer, mesh.vertexBuffer, {bufferCopyRegion});
    }

    void updateMesh(std::size_t tileCount, Mesh& mesh, entt::registry& registry, renderer::Buffer& stagingBuffer, renderer::CommandBuffer& commandBuffer, std::size_t stagingBufferOffset) {
        if (!mesh.textureBuffer || !mesh.transformBuffer || !mesh.vertexBuffer || !stagingBuffer || tileCount == 0 || !renderer::CommandBuffer::capturing(commandBuffer)) {
            return;
        }

        std::size_t totalSize = renderer::Buffer::size(mesh.textureBuffer) + renderer::Buffer::size(mesh.transformBuffer);
        std::size_t textureBytes = tileCount * sizeof(MeshTexture);
        std::size_t transformBytes = tileCount * sizeof(MeshTransform);

        auto mapping = renderer::Buffer::map(stagingBuffer, totalSize, stagingBufferOffset);

        auto* texturePointer = reinterpret_cast<MeshTexture*>(mapping.data.data());
        auto* transformPointer = reinterpret_cast<MeshTransform*>(reinterpret_cast<std::byte*>(mapping.data.data()) + textureBytes);

        std::size_t i = 0;

        for (auto [entity, meshTexture, transform] : registry.view<MeshTexture, MeshTransform>().each()) {
            texturePointer[i] = meshTexture;
            transformPointer[i] = transform;
            i++;
        }

        renderer::Buffer::unmap(stagingBuffer, mapping);

        renderer::BufferCopyRegion textureBufferCopyRegion = {
            .sourceOffsetBytes = stagingBufferOffset,
            .destinationOffsetBytes = 0,
            .sizeBytes = textureBytes,
        };

        renderer::BufferCopyRegion transformBufferCopyRegion = {
            .sourceOffsetBytes = stagingBufferOffset + textureBytes,
            .destinationOffsetBytes = 0,
            .sizeBytes = transformBytes,
        };

        stagingBufferOffset += totalSize;

        renderer::CommandBuffer::copyBuffer(commandBuffer, stagingBuffer, mesh.textureBuffer, {textureBufferCopyRegion});
        renderer::CommandBuffer::copyBuffer(commandBuffer, stagingBuffer, mesh.transformBuffer, {transformBufferCopyRegion});
    }

    void destroyMesh(Mesh& mesh) {
        if (mesh.vertexBuffer) {
            renderer::Buffer::destroy(mesh.vertexBuffer);
        }

        if (mesh.transformBuffer) {
            renderer::Buffer::destroy(mesh.transformBuffer);
        }

        if (mesh.textureBuffer) {
            renderer::Buffer::destroy(mesh.textureBuffer);
        }
    }
}