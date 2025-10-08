#pragma once

#include <data/unique.hpp>

#include <app/window.hpp>

#include <renderer/resources/buffer.hpp>

#include <renderer/commands/buffer.hpp>

#include <glm/glm.hpp>

namespace game {
    struct Controller;
    struct MovableBody;
    struct BoxCollisionResult;

    enum class CharacterState {
        ACCELERATING,
        SLOWING,
        AIRBORNE,
        IDLE,
    };

    struct CharacterVertex {
        glm::vec2 position;
        glm::vec2 texCoord;
    };

    struct CharacterInstance {
        glm::vec3 position = {0.0f, 0.0f, 0.0f};
        glm::vec2 scale = {1.0f, 1.0f};
        glm::vec2 texOffset = {0.0f, 0.0f};
    };

    struct CharacterMesh {
        data::Unique<renderer::Buffer> vertexBuffer;
        data::Unique<renderer::Buffer> instanceBuffer;
    };

    struct Character {
        float speed = 0.0f;
        bool accelerating = false;

        CharacterState state = CharacterState::IDLE;
    };

    void createCharacters(CharacterMesh& mesh, std::span<CharacterInstance> instances, renderer::Device& device, renderer::Buffer& stagingBuffer, std::uint64_t& stagingBufferOffset, renderer::CommandBuffer& transferBuffer);
    void updateCharacters(CharacterMesh& mesh, std::span<CharacterInstance> instances, renderer::Buffer& stagingBuffer, std::uint64_t& stagingBufferOffset, renderer::CommandBuffer& transferBuffer);
    void renderCharacters(CharacterMesh& mesh, std::span<CharacterInstance> instances, renderer::CommandBuffer& commandBuffer);

    void setCharacterState(Character& character, const MovableBody& body, const BoxCollisionResult& collisionResult);
}