#pragma once

#include <data/unique.hpp>

#include <app/window.hpp>

#include <renderer/resources/buffer.hpp>

#include <renderer/commands/buffer.hpp>

#include <glm/glm.hpp>

namespace game {
    struct SettingsConfig;

    struct Controller;

    struct CharacterVertex {
        glm::vec2 position;
        glm::vec2 texCoord;
    };

    struct Character {
        float speed = 1.0f;

        glm::vec2 position = {0.0f, 0.0f};
        glm::vec2 orientation = {1.0f, 0.0f};
        glm::vec2 velocity = {0.0f, 0.0f};
        glm::mat4 model = {1.0f};

        data::Unique<renderer::Buffer> vertexBuffer;
        data::Unique<renderer::Buffer> indexBuffer;
    };

    void createCharacter(Character& character, renderer::Device& device, renderer::Buffer& stagingBuffer, std::uint64_t& stagingBufferOffset, renderer::CommandBuffer& transferBuffer);
    void updateCharacter(Character& character, float deltaTime);
    void renderCharacter(Character& character, renderer::CommandBuffer& commandBuffer, renderer::PipelineLayout& pipelineLayout);

    void updateCharacterVelocity(Character& character, Controller& controller, app::WindowKeyPressedEventInfo& eventInfo);
    void updateCharacterVelocity(Character& character, Controller& controller, app::WindowKeyReleasedEventInfo& eventInfo);
}