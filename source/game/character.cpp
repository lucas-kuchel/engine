#include <game/character.hpp>
#include <game/controller.hpp>
#include <game/settings.hpp>

#include <renderer/device.hpp>
#include <renderer/queue.hpp>

#include <renderer/resources/buffer.hpp>
#include <renderer/resources/fence.hpp>
#include <renderer/resources/sampler.hpp>

#include <cstring>

#include <glm/gtc/matrix_transform.hpp>

namespace game {
    void createCharacter(Character& character, renderer::Device& device, renderer::Buffer& stagingBuffer, std::uint64_t& stagingBufferOffset, renderer::CommandBuffer& transferBuffer) {
        std::array<CharacterVertex, 4> vertices = {
            CharacterVertex({0.5, -1.0}, {1.0, 0.0}),
            CharacterVertex({-0.5, -1.0}, {0.0, 0.0}),
            CharacterVertex({-0.5, 1.0}, {0.0, 2.0}),
            CharacterVertex({0.5, 1.0}, {1.0, 2.0}),
        };

        std::array<std::uint32_t, 6> indices = {
            0,
            1,
            2,
            0,
            2,
            3,
        };

        renderer::BufferCreateInfo vertexBufferCreateInfo = {
            .device = device,
            .memoryType = renderer::MemoryType::DEVICE_LOCAL,
            .usageFlags = renderer::BufferUsageFlags::VERTEX | renderer::BufferUsageFlags::TRANSFER_DESTINATION,
            .sizeBytes = vertices.size() * sizeof(CharacterVertex),
        };

        renderer::BufferCreateInfo indexBufferCreateInfo = {
            .device = device,
            .memoryType = renderer::MemoryType::DEVICE_LOCAL,
            .usageFlags = renderer::BufferUsageFlags::INDEX | renderer::BufferUsageFlags::TRANSFER_DESTINATION,
            .sizeBytes = indices.size() * sizeof(std::uint32_t),
        };

        character.vertexBuffer = data::makeUnique<renderer::Buffer>(vertexBufferCreateInfo);
        character.indexBuffer = data::makeUnique<renderer::Buffer>(indexBufferCreateInfo);

        auto stagingBufferData = stagingBuffer.map(character.vertexBuffer->size() + character.indexBuffer->size(), stagingBufferOffset);

        std::memcpy(stagingBufferData.data(), vertices.data(), character.vertexBuffer->size());
        std::memcpy(stagingBufferData.data() + character.vertexBuffer->size(), indices.data(), character.indexBuffer->size());

        stagingBuffer.unmap();

        renderer::BufferCopyRegion vertexBufferCopyRegion = {
            .sourceOffsetBytes = stagingBufferOffset,
            .destinationOffsetBytes = 0,
            .sizeBytes = character.vertexBuffer->size(),
        };

        renderer::BufferCopyRegion indexBufferCopyRegion = {
            .sourceOffsetBytes = stagingBufferOffset + character.vertexBuffer->size(),
            .destinationOffsetBytes = 0,
            .sizeBytes = character.indexBuffer->size(),
        };

        stagingBufferOffset += character.vertexBuffer->size() + character.indexBuffer->size();

        transferBuffer.copyBuffer(stagingBuffer, character.vertexBuffer.ref(), {vertexBufferCopyRegion});
        transferBuffer.copyBuffer(stagingBuffer, character.indexBuffer.ref(), {indexBufferCopyRegion});
    }

    void updateCharacter(Character& character, float deltaTime) {
        if (glm::length(character.velocity) > 0.0f) {
            character.velocity = glm::normalize(character.velocity);
            character.position += character.velocity * character.speed * deltaTime;

            character.orientation = character.velocity;
        }

        glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(character.orientation.x, 1.0f, 1.0f));

        character.model = glm::mat4(1.0f);
        character.model = glm::translate(character.model, glm::vec3(character.position, 0.0f)) * scale;
    }

    void renderCharacter(Character& character, renderer::CommandBuffer& commandBuffer, renderer::PipelineLayout& pipelineLayout) {
        std::span<std::uint8_t> pushConstant = {reinterpret_cast<std::uint8_t*>(&character.model), sizeof(glm::mat4)};

        commandBuffer.pushConstants(pipelineLayout, renderer::DescriptorShaderStageFlags::VERTEX, pushConstant, 0);
        commandBuffer.bindVertexBuffers({character.vertexBuffer.ref()}, {0}, 0);
        commandBuffer.bindIndexBuffer(character.indexBuffer.ref(), 0, renderer::IndexType::UINT32);
        commandBuffer.drawIndexed(6, 1, 0, 0, 0);
    }

    void updateCharacterVelocity(Character& character, Controller& controller, app::WindowKeyPressedEventInfo& eventInfo) {
        if (eventInfo.key == controller.leftBinding) {
            character.velocity.x -= 1.0f;
        }
        else if (eventInfo.key == controller.rightBinding) {
            character.velocity.x += 1.0f;
        }
    }

    void updateCharacterVelocity(Character& character, Controller& controller, app::WindowKeyReleasedEventInfo& eventInfo) {
        if (eventInfo.key == controller.leftBinding) {
            character.velocity.x += 1.0f;
        }
        else if (eventInfo.key == controller.rightBinding) {
            character.velocity.x -= 1.0f;
        }
    }
}