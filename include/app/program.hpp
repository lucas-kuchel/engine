#pragma once

#include <app/configuration.hpp>
#include <app/context.hpp>
#include <app/window.hpp>

#include <renderer/buffer.hpp>
#include <renderer/command_buffer.hpp>
#include <renderer/command_pool.hpp>
#include <renderer/device.hpp>
#include <renderer/fence.hpp>
#include <renderer/framebuffer.hpp>
#include <renderer/image.hpp>
#include <renderer/image_view.hpp>
#include <renderer/instance.hpp>
#include <renderer/pipeline.hpp>
#include <renderer/render_pass.hpp>
#include <renderer/sampler.hpp>
#include <renderer/semaphore.hpp>
#include <renderer/shader_module.hpp>
#include <renderer/surface.hpp>
#include <renderer/swapchain.hpp>

#include <game/camera.hpp>
#include <game/character.hpp>
#include <game/controller.hpp>
#include <game/map.hpp>
#include <game/settings.hpp>

#include <array>
#include <chrono>
#include <memory>

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

        std::unique_ptr<Context> context_;
        std::unique_ptr<Window> window_;

        renderer::Instance instance_;
        renderer::Surface surface_;
        renderer::Device device_;
        renderer::Swapchain swapchain_;
        renderer::Queue graphicsQueue_;
        renderer::Queue transferQueue_;
        renderer::Queue presentQueue_;
        renderer::Pipeline basicPipeline_;
        renderer::CommandBuffer transferCommandBuffer_;
        renderer::DescriptorSet basicDescriptorSet_;
        renderer::RenderPass renderPass_;
        renderer::CommandPool graphicsCommandPool_;
        renderer::CommandPool transferCommandPool_;
        renderer::PipelineLayout basicPipelineLayout_;
        renderer::DescriptorPool descriptorPool_;
        renderer::DescriptorSetLayout descriptorSetLayout_;
        renderer::Fence stagingBufferFence_;
        renderer::Semaphore stagingBufferSemaphore_;
        renderer::Image tilemapImage_;
        renderer::ImageView tilemapImageView_;
        renderer::Sampler sampler_;
        renderer::Buffer stagingBuffer_;

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

        bool resized_ = false;

        game::CharacterMesh characterMesh_;
        game::TileMesh tileMesh_;
        game::Controller controller_;
        game::Camera camera_;
        game::Settings settings_;
        game::Map map_;

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