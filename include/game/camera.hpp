#pragma once

#include <data/unique.hpp>

#include <app/window.hpp>

#include <renderer/resources/buffer.hpp>

#include <renderer/commands/buffer.hpp>

#include <renderer/swapchain.hpp>

#include <glm/glm.hpp>

namespace game {
    struct SettingsConfig;
    class Player;

    class Camera {
    public:
        Camera(SettingsConfig& settings, renderer::Device& device, renderer::CommandBuffer& transferCommandBuffer, renderer::Queue& transferQueue, renderer::Swapchain& swapchain);
        ~Camera();

        void update();

        void slowMoveToPlayer(float deltaTime, const Player& player);

        renderer::DescriptorSet& descriptorSet();
        renderer::DescriptorSetLayout& descriptorSetLayout();

    private:
        float rotation_ = 0.0f;
        float screenSizeWorldUnits_ = 20.0f;

        glm::vec2 position_ = {0.0f, 0.0f};
        glm::mat4 projection_ = {1.0f};
        glm::mat4 view_ = {1.0f};

        data::Unique<renderer::DescriptorSetLayout> descriptorSetLayout_;
        data::Unique<renderer::DescriptorPool> descriptorPool_;

        data::Unique<renderer::Buffer> cameraBuffer_;

        data::NullableRef<renderer::DescriptorSet> descriptorSet_;

        std::vector<renderer::DescriptorSet> descriptorSets_;

        renderer::Device& device_;
        renderer::CommandBuffer& transferCommandBuffer_;
        renderer::Queue& transferQueue_;
        renderer::Swapchain& swapchain_;

        SettingsConfig& settings_;
    };
}