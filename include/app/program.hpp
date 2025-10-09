#pragma once

#include <data/unique.hpp>

#include <app/context.hpp>
#include <app/window.hpp>

#include <renderer/device.hpp>
#include <renderer/instance.hpp>
#include <renderer/surface.hpp>
#include <renderer/swapchain.hpp>

#include <renderer/commands/buffer.hpp>
#include <renderer/commands/pool.hpp>

#include <renderer/resources/buffer.hpp>
#include <renderer/resources/framebuffer.hpp>
#include <renderer/resources/image.hpp>
#include <renderer/resources/pass.hpp>
#include <renderer/resources/pipeline.hpp>
#include <renderer/resources/sampler.hpp>
#include <renderer/resources/shader.hpp>

#include <game/camera.hpp>
#include <game/character.hpp>
#include <game/controller.hpp>
#include <game/map.hpp>
#include <game/settings.hpp>

#include <array>
#include <chrono>

#include <glm/glm.hpp>

namespace app {
    struct Counter {
        std::uint32_t count = 0;
        std::uint32_t index = 0;
    };

    class Program {
    public:
        Program();
        ~Program();

    private:
        void manageEvents(bool& running);
        void acquireImage(bool& resized);
        void presentImage();

        void run();

        void start();
        void update();
        void render();
        void close();

        void createBasicPipelineResources();

        static std::size_t keyIndex(Key key);

        data::Unique<Context> context_;
        data::Unique<Window> window_;

        data::Unique<renderer::Instance> instance_;
        data::Unique<renderer::Surface> surface_;
        data::Unique<renderer::Device> device_;
        data::Unique<renderer::Swapchain> swapchain_;
        data::Unique<renderer::RenderPass> renderPass_;
        data::Unique<renderer::CommandPool> graphicsCommandPool_;
        data::Unique<renderer::CommandPool> transferCommandPool_;
        data::Unique<renderer::PipelineLayout> basicPipelineLayout_;
        data::Unique<renderer::DescriptorPool> descriptorPool_;
        data::Unique<renderer::DescriptorSetLayout> descriptorSetLayout_;
        data::Unique<renderer::Fence> stagingBufferFence_;
        data::Unique<renderer::Semaphore> stagingBufferSemaphore_;
        data::Unique<renderer::Image> tilemapImage_;
        data::Unique<renderer::ImageView> tilemapImageView_;
        data::Unique<renderer::Sampler> sampler_;

        data::NullableRef<renderer::Queue> graphicsQueue_;
        data::NullableRef<renderer::Queue> transferQueue_;
        data::NullableRef<renderer::Queue> presentQueue_;
        data::NullableRef<renderer::Pipeline> basicPipeline_;
        data::NullableRef<renderer::CommandBuffer> transferCommandBuffer_;
        data::NullableRef<renderer::DescriptorSet> basicDescriptorSet_;

        std::vector<renderer::Fence> inFlightFences_;
        std::vector<renderer::Semaphore> acquireSemaphores_;
        std::vector<renderer::Semaphore> presentSemaphores_;
        std::vector<renderer::Framebuffer> framebuffers_;
        std::vector<renderer::CommandBuffer> commandBuffers_;
        std::vector<renderer::Pipeline> pipelines_;
        std::vector<renderer::CommandBuffer> transferCommandBuffers_;
        std::vector<renderer::DescriptorSet> descriptorSets_;
        std::vector<renderer::Image> depthImages_;
        std::vector<renderer::ImageView> depthImageViews_;

        Counter imageCounter_;
        Counter frameCounter_;

        bool explicitSwapchainRecreate_ = false;

        game::CharacterMesh characterMesh_;
        game::TileMesh tileMesh_;
        game::Controller controller_;
        game::Camera camera_;
        game::Settings settings_;
        game::Map map_;

        data::Unique<renderer::Buffer> stagingBuffer_;

        std::vector<game::Character> characters_;
        std::vector<game::MovableBody> characterMovableBodies_;
        std::vector<game::BoxCollider> characterColliders_;
        std::vector<game::BoxCollisionResult> characterCollisionResults_;
        std::vector<game::CharacterInstance> characterInstances_;

        std::uint32_t focusedCharacterIndex_ = 0;

        std::chrono::time_point<std::chrono::high_resolution_clock> lastFrameTime_;

        std::array<bool, 93> keysPressed_ = {false};
        std::array<bool, 93> keysHeld_ = {false};
        std::array<bool, 93> keysReleased_ = {false};
    };
}