#pragma once

#include <app/window.hpp>

#include <renderer/buffer.hpp>
#include <renderer/command_buffer.hpp>

namespace game {
    struct Controller;
    struct MovableBody;
    struct CollisionResult;

    struct alignas(32) CharacterInstance {
        glm::vec2 texturePosition = {0.0f, 0.0f};
        glm::vec2 textureExtent = {1.0f, 1.0f};
        glm::vec2 textureOffset = {0.0f, 0.0f};
        glm::vec2 textureScale = {1.0f, 1.0f};
    };

    struct CharacterMesh {
        renderer::Buffer vertexBuffer;
        renderer::Buffer instanceBuffer;
        renderer::Buffer modelBuffer;
    };

    struct Character {
        float baseSpeed = 0.0f;
        float sprintMultiplier = 0.0f;
        float jumpForce = 0.0f;
        float animationTime = 0.0f;
        std::uint32_t animationIndex = 0;

        bool sprinting = false;
    };

    void createCharacterInstances(CharacterMesh& mesh, std::span<CharacterInstance> instances, std::span<glm::mat4> models, renderer::Device& device, renderer::Buffer& stagingBuffer, std::uint64_t& stagingBufferOffset, renderer::CommandBuffer& transferBuffer);
    void updateCharacterInstances(CharacterMesh& mesh, std::span<CharacterInstance> instances, std::span<glm::mat4> models, renderer::Buffer& stagingBuffer, std::uint64_t& stagingBufferOffset, renderer::CommandBuffer& transferBuffer);
    void renderCharacterInstances(CharacterMesh& mesh, std::uint32_t count, renderer::CommandBuffer& commandBuffer);
    void destroyCharacterInstances(CharacterMesh& mesh);
    float currentCharacterSpeed(const Character& character);
}