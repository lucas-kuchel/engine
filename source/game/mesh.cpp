#include <game/mesh.hpp>

namespace game {
    void createMesh(renderer::Device& device, Mesh& mesh, std::size_t instanceCount) {
        if (mesh.textureBuffer || mesh.transformBuffer || mesh.vertexBuffer || instanceCount == 0) {
            return;
        }

        renderer::BufferCreateInfo createInfo = {
            .device = device,
            .memoryType = renderer::MemoryType::DEVICE_LOCAL,
            .sizeBytes = instanceCount,
            .usageFlags = renderer::BufferUsageFlags::VERTEX | renderer::BufferUsageFlags::TRANSFER_DESTINATION,
        };
    }

    void updateMesh(Mesh& mesh, std::size_t instanceCount, std::span<std::uint8_t> data, renderer::CommandBuffer& commandBuffer, renderer::Buffer& stagingBuffer, std::size_t stagingBufferOffset) {
    }

    void deleteMesh(Mesh& mesh) {
    }
}