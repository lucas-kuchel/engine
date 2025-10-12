#include <game/character.hpp>
#include <game/controller.hpp>
#include <game/physics.hpp>
#include <game/settings.hpp>

#include <renderer/buffer.hpp>
#include <renderer/device.hpp>
#include <renderer/fence.hpp>
#include <renderer/queue.hpp>
#include <renderer/sampler.hpp>

#include <array>
#include <cstring>

#include <glm/gtc/matrix_transform.hpp>

namespace game {
    void createCharacterInstances(CharacterMesh& mesh, std::span<CharacterInstance> instances, renderer::Device& device, renderer::Buffer& stagingBuffer, std::uint64_t& stagingBufferOffset, renderer::CommandBuffer& transferBuffer) {
        std::array<CharacterVertex, 4> vertices = {
            CharacterVertex({0.5, -0.5}, {0.5, 0.0}),
            CharacterVertex({-0.5, -0.5}, {0.0, 0.0}),
            CharacterVertex({0.5, 0.5}, {0.5, 0.5}),
            CharacterVertex({-0.5, 0.5}, {0.0, 0.5}),
        };

        renderer::BufferCreateInfo vertexBufferCreateInfo = {
            .device = device,
            .memoryType = renderer::MemoryType::DEVICE_LOCAL,
            .usageFlags = renderer::BufferUsageFlags::VERTEX | renderer::BufferUsageFlags::TRANSFER_DESTINATION,
            .sizeBytes = vertices.size() * sizeof(CharacterVertex),
        };

        renderer::BufferCreateInfo instanceBufferCreateInfo = {
            .device = device,
            .memoryType = renderer::MemoryType::DEVICE_LOCAL,
            .usageFlags = renderer::BufferUsageFlags::VERTEX | renderer::BufferUsageFlags::TRANSFER_DESTINATION,
            .sizeBytes = instances.size() * sizeof(CharacterInstance),
        };

        mesh.vertexBuffer = renderer::Buffer::create(vertexBufferCreateInfo);
        mesh.instanceBuffer = renderer::Buffer::create(instanceBufferCreateInfo);

        std::uint64_t totalSize = renderer::Buffer::size(mesh.vertexBuffer) + renderer::Buffer::size(mesh.instanceBuffer);
        auto mapping = renderer::Buffer::map(stagingBuffer, totalSize, stagingBufferOffset);

        std::memcpy(mapping.data.data(), vertices.data(), renderer::Buffer::size(mesh.vertexBuffer));
        std::memcpy(mapping.data.data() + renderer::Buffer::size(mesh.vertexBuffer), instances.data(), renderer::Buffer::size(mesh.instanceBuffer));

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

    void updateCharacterInstances(CharacterMesh& mesh, std::span<CharacterInstance> instances, renderer::Buffer& stagingBuffer, std::uint64_t& stagingBufferOffset, renderer::CommandBuffer& transferBuffer) {
        std::size_t instanceBufferSize = renderer::Buffer::size(mesh.instanceBuffer);

        auto mapping = renderer::Buffer::map(stagingBuffer, instanceBufferSize, stagingBufferOffset);

        std::memcpy(mapping.data.data(), instances.data(), instanceBufferSize);

        renderer::Buffer::unmap(stagingBuffer, mapping);

        renderer::BufferCopyRegion instanceBufferCopyRegion = {
            .sourceOffsetBytes = stagingBufferOffset,
            .destinationOffsetBytes = 0,
            .sizeBytes = instanceBufferSize,
        };

        stagingBufferOffset += instanceBufferSize;

        renderer::CommandBuffer::copyBuffer(transferBuffer, stagingBuffer, mesh.instanceBuffer, {instanceBufferCopyRegion});
    }

    void renderCharacterInstances(CharacterMesh& mesh, std::span<CharacterInstance> instances, renderer::CommandBuffer& commandBuffer) {
        renderer::CommandBuffer::bindVertexBuffers(commandBuffer, {mesh.vertexBuffer, mesh.instanceBuffer}, {0, 0}, 0);
        renderer::CommandBuffer::draw(commandBuffer, 4, static_cast<std::uint32_t>(instances.size()), 0, 0);
    }

    void destroyCharacterInstances(CharacterMesh& mesh) {
        if (mesh.instanceBuffer) {
            renderer::Buffer::destroy(mesh.instanceBuffer);
        }

        if (mesh.vertexBuffer) {
            renderer::Buffer::destroy(mesh.vertexBuffer);
        }
    }

    float currentCharacterSpeed(const Character& character) {
        return character.sprinting ? character.baseSpeed * character.sprintMultiplier : character.baseSpeed;
    }
}