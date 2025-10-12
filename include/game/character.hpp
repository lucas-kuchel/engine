#pragma once

#include <app/window.hpp>

#include <renderer/buffer.hpp>
#include <renderer/command_buffer.hpp>

namespace game {
    struct Controller;
    struct MovableBody;
    struct BoxCollisionResult;

    enum class CharacterFacing {
        LEFT,
        RIGHT,
    };

    struct CharacterVertex {
        glm::vec2 position;
        glm::vec2 texCoord;
    };

    struct alignas(32) CharacterInstance {
        glm::vec3 position = {0.0f, 0.0f, 0.0f};
        glm::vec2 scale = {1.0f, 1.0f};
        glm::vec2 texOffset = {0.0f, 0.0f};
    };

    struct CharacterMesh {
        renderer::Buffer vertexBuffer;
        renderer::Buffer instanceBuffer;
    };

    struct Character {
        float baseSpeed = 0.0f;
        float sprintMultiplier = 0.0f;
        float jumpForce = 0.0f;

        bool sprinting = false;

        CharacterFacing facing = CharacterFacing::RIGHT;
    };

    void createCharacterInstances(CharacterMesh& mesh, std::span<CharacterInstance> instances, renderer::Device& device, renderer::Buffer& stagingBuffer, std::uint64_t& stagingBufferOffset, renderer::CommandBuffer& transferBuffer);
    void updateCharacterInstances(CharacterMesh& mesh, std::span<CharacterInstance> instances, renderer::Buffer& stagingBuffer, std::uint64_t& stagingBufferOffset, renderer::CommandBuffer& transferBuffer);
    void renderCharacterInstances(CharacterMesh& mesh, std::span<CharacterInstance> instances, renderer::CommandBuffer& commandBuffer);
    void destroyCharacterInstances(CharacterMesh& mesh);

    float currentCharacterSpeed(const Character& character);
}