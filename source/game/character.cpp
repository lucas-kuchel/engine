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
    void createCharacterInstances(CharacterMesh& mesh, std::span<CharacterInstance> instances, std::span<glm::mat4> models, renderer::Device& device, renderer::Buffer& stagingBuffer, std::uint64_t& stagingBufferOffset, renderer::CommandBuffer& transferBuffer) {
        std::array<glm::vec3, 4> vertices = {
            glm::vec3{0.5, 0.0, 0.5},
            glm::vec3{-0.5, 0.0, 0.5},
            glm::vec3{0.5, 0.0, -0.5},
            glm::vec3{-0.5, 0.0, -0.5},
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
            .sizeBytes = instances.size() * sizeof(CharacterInstance),
        };

        renderer::BufferCreateInfo modelBufferCreateInfo = {
            .device = device,
            .memoryType = renderer::MemoryType::DEVICE_LOCAL,
            .usageFlags = renderer::BufferUsageFlags::VERTEX | renderer::BufferUsageFlags::TRANSFER_DESTINATION,
            .sizeBytes = models.size() * sizeof(glm::mat4),
        };

        mesh.vertexBuffer = renderer::Buffer::create(vertexBufferCreateInfo);
        mesh.instanceBuffer = renderer::Buffer::create(instanceBufferCreateInfo);
        mesh.modelBuffer = renderer::Buffer::create(modelBufferCreateInfo);

        std::uint64_t teritarySize = renderer::Buffer::size(mesh.vertexBuffer);
        std::uint64_t secondarySize = teritarySize + renderer::Buffer::size(mesh.instanceBuffer);
        std::uint64_t totalSize = secondarySize + renderer::Buffer::size(mesh.modelBuffer);

        auto mapping = renderer::Buffer::map(stagingBuffer, totalSize, stagingBufferOffset);

        std::memcpy(mapping.data.data(), vertices.data(), renderer::Buffer::size(mesh.vertexBuffer));
        std::memcpy(mapping.data.data() + secondarySize, models.data(), renderer::Buffer::size(mesh.modelBuffer));
        std::memcpy(mapping.data.data() + teritarySize, instances.data(), renderer::Buffer::size(mesh.instanceBuffer));

        renderer::Buffer::unmap(stagingBuffer, mapping);

        renderer::BufferCopyRegion vertexBufferCopyRegion = {
            .sourceOffsetBytes = stagingBufferOffset,
            .destinationOffsetBytes = 0,
            .sizeBytes = renderer::Buffer::size(mesh.vertexBuffer),
        };

        renderer::BufferCopyRegion modelBufferCopyRegion = {
            .sourceOffsetBytes = stagingBufferOffset + secondarySize,
            .destinationOffsetBytes = 0,
            .sizeBytes = renderer::Buffer::size(mesh.modelBuffer),
        };

        renderer::BufferCopyRegion instanceBufferCopyRegion = {
            .sourceOffsetBytes = stagingBufferOffset + teritarySize,
            .destinationOffsetBytes = 0,
            .sizeBytes = renderer::Buffer::size(mesh.instanceBuffer),
        };

        stagingBufferOffset += totalSize;

        renderer::CommandBuffer::copyBuffer(transferBuffer, stagingBuffer, mesh.vertexBuffer, {vertexBufferCopyRegion});
        renderer::CommandBuffer::copyBuffer(transferBuffer, stagingBuffer, mesh.modelBuffer, {modelBufferCopyRegion});
        renderer::CommandBuffer::copyBuffer(transferBuffer, stagingBuffer, mesh.instanceBuffer, {instanceBufferCopyRegion});
    }

    void updateCharacterInstances(CharacterMesh& mesh, std::span<glm::mat4> models, renderer::Buffer& stagingBuffer, std::uint64_t& stagingBufferOffset, renderer::CommandBuffer& transferBuffer) {
        std::size_t instanceBufferSize = renderer::Buffer::size(mesh.modelBuffer);

        auto mapping = renderer::Buffer::map(stagingBuffer, instanceBufferSize, stagingBufferOffset);

        std::memcpy(mapping.data.data(), models.data(), instanceBufferSize);

        renderer::Buffer::unmap(stagingBuffer, mapping);

        renderer::BufferCopyRegion instanceBufferCopyRegion = {
            .sourceOffsetBytes = stagingBufferOffset,
            .destinationOffsetBytes = 0,
            .sizeBytes = instanceBufferSize,
        };

        stagingBufferOffset += instanceBufferSize;

        renderer::CommandBuffer::copyBuffer(transferBuffer, stagingBuffer, mesh.modelBuffer, {instanceBufferCopyRegion});
    }

    void renderCharacterInstances(CharacterMesh& mesh, std::uint32_t count, renderer::CommandBuffer& commandBuffer) {
        renderer::CommandBuffer::bindVertexBuffers(commandBuffer, {mesh.vertexBuffer, mesh.instanceBuffer, mesh.modelBuffer}, {0, 0, 0}, 0);
        renderer::CommandBuffer::draw(commandBuffer, 4, count, 0, 0);
    }

    void destroyCharacterInstances(CharacterMesh& mesh) {
        if (mesh.instanceBuffer) {
            renderer::Buffer::destroy(mesh.instanceBuffer);
        }

        if (mesh.vertexBuffer) {
            renderer::Buffer::destroy(mesh.vertexBuffer);
        }

        if (mesh.modelBuffer) {
            renderer::Buffer::destroy(mesh.modelBuffer);
        }
    }

    float currentCharacterSpeed(const Character& character) {
        return character.sprinting ? character.baseSpeed * character.sprintMultiplier : character.baseSpeed;
    }
}