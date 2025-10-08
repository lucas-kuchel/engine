#include <game/character.hpp>
#include <game/controller.hpp>
#include <game/physics.hpp>
#include <game/settings.hpp>

#include <renderer/device.hpp>
#include <renderer/queue.hpp>

#include <renderer/resources/buffer.hpp>
#include <renderer/resources/fence.hpp>
#include <renderer/resources/sampler.hpp>

#include <cstring>

#include <glm/gtc/matrix_transform.hpp>

namespace game {
    void createCharacters(CharacterMesh& mesh, std::span<CharacterInstance> instances, renderer::Device& device, renderer::Buffer& stagingBuffer, std::uint64_t& stagingBufferOffset, renderer::CommandBuffer& transferBuffer) {
        std::array<CharacterVertex, 4> vertices = {
            CharacterVertex({0.5, -0.5}, {1.0, 0.0}),
            CharacterVertex({-0.5, -0.5}, {0.0, 0.0}),
            CharacterVertex({0.5, 0.5}, {1.0, 1.0}),
            CharacterVertex({-0.5, 0.5}, {0.0, 1.0}),
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

        mesh.vertexBuffer = data::makeUnique<renderer::Buffer>(vertexBufferCreateInfo);
        mesh.instanceBuffer = data::makeUnique<renderer::Buffer>(instanceBufferCreateInfo);

        std::uint64_t totalSize = mesh.vertexBuffer->size() + mesh.instanceBuffer->size();
        std::span<std::uint8_t> stagingBufferData = stagingBuffer.map(totalSize, stagingBufferOffset);

        std::memcpy(stagingBufferData.data(), vertices.data(), mesh.vertexBuffer->size());
        std::memcpy(stagingBufferData.data() + mesh.vertexBuffer->size(), instances.data(), mesh.instanceBuffer->size());

        stagingBuffer.unmap();

        renderer::BufferCopyRegion vertexBufferCopyRegion = {
            .sourceOffsetBytes = stagingBufferOffset,
            .destinationOffsetBytes = 0,
            .sizeBytes = mesh.vertexBuffer->size(),
        };

        renderer::BufferCopyRegion instanceBufferCopyRegion = {
            .sourceOffsetBytes = stagingBufferOffset + mesh.vertexBuffer->size(),
            .destinationOffsetBytes = 0,
            .sizeBytes = mesh.instanceBuffer->size(),
        };

        stagingBufferOffset += mesh.vertexBuffer->size() + mesh.instanceBuffer->size();

        transferBuffer.copyBuffer(stagingBuffer, mesh.vertexBuffer.ref(), {vertexBufferCopyRegion});
        transferBuffer.copyBuffer(stagingBuffer, mesh.instanceBuffer.ref(), {instanceBufferCopyRegion});
    }

    void updateCharacters(CharacterMesh& mesh, std::span<CharacterInstance> instances, renderer::Buffer& stagingBuffer, std::uint64_t& stagingBufferOffset, renderer::CommandBuffer& transferBuffer) {
        std::span<std::uint8_t> stagingBufferData = stagingBuffer.map(mesh.instanceBuffer->size(), stagingBufferOffset);

        std::memcpy(stagingBufferData.data(), instances.data(), mesh.instanceBuffer->size());

        stagingBuffer.unmap();

        renderer::BufferCopyRegion instanceBufferCopyRegion = {
            .sourceOffsetBytes = stagingBufferOffset,
            .destinationOffsetBytes = 0,
            .sizeBytes = mesh.instanceBuffer->size(),
        };

        stagingBufferOffset += mesh.instanceBuffer->size();

        transferBuffer.copyBuffer(stagingBuffer, mesh.instanceBuffer.ref(), {instanceBufferCopyRegion});
    }

    void renderCharacters(CharacterMesh& character, std::span<CharacterInstance> instances, renderer::CommandBuffer& commandBuffer) {
        commandBuffer.bindVertexBuffers({character.vertexBuffer.ref(), character.instanceBuffer.ref()}, {0, 0}, 0);
        commandBuffer.draw(4, static_cast<std::uint32_t>(instances.size()), 0, 0);
    }

    void setCharacterState(Character& character, const MovableBody& body, const CollisionResult& collisionResult) {
        if (!collisionResult.collided) {
            character.state = CharacterState::AIRBORNE;
        }
        else if (body.acceleration == glm::vec2{0.0f, 0.0f}) {
            character.state = CharacterState::IDLE;
        }
        else if (character.accelerating) {
            character.state = CharacterState::ACCELERATING;
        }
        else {
            character.state = CharacterState::SLOWING;
        }
    }
}