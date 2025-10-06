#pragma once

#include <data/unique.hpp>

#include <app/window.hpp>

#include <renderer/resources/buffer.hpp>

#include <renderer/commands/buffer.hpp>

#include <glm/glm.hpp>

namespace game {
    struct SettingsConfig;

    struct Vertex {
        glm::vec2 position;
        glm::vec2 texCoord;
    };

    struct Triangle {
        glm::uvec3 indices;
    };

    class Player {
    public:
        Player(SettingsConfig& settings, renderer::Device& device, renderer::CommandBuffer& transferCommandBuffer, renderer::Queue& transferCommandQueue);
        ~Player();

        void beginMove(const app::WindowKeyPressedEventInfo& pressEvent);
        void stopMove(const app::WindowKeyReleasedEventInfo& releaseEvent);

        void update(float deltaTime);
        void render(renderer::CommandBuffer& graphicsCommandBuffer, renderer::PipelineLayout& layout);

        renderer::DescriptorSet& descriptorSet();
        renderer::DescriptorSetLayout& descriptorSetLayout();
        renderer::PushConstantInputInfo pushConstant();

        glm::vec2 position() const;
        glm::vec2 orientation() const;

    private:
        glm::vec2 position_ = {0.0f, 0.0f};
        glm::vec2 orientation_ = {0.0f, 0.0f};

        float mirrorX_ = 1.0f;

        std::int32_t deltaX_ = 0;

        glm::mat4 model_ = {1.0f};

        data::Unique<renderer::Image> image_;
        data::Unique<renderer::ImageView> imageView_;
        data::Unique<renderer::Sampler> sampler_;

        data::Unique<renderer::Buffer> vertexBuffer_;
        data::Unique<renderer::Buffer> indexBuffer_;

        data::Unique<renderer::DescriptorSetLayout> descriptorSetLayout_;
        data::Unique<renderer::DescriptorPool> descriptorPool_;

        data::NullableRef<renderer::DescriptorSet> descriptorSet_;

        std::vector<renderer::DescriptorSet> descriptorSets_;

        SettingsConfig& settings_;
    };
}