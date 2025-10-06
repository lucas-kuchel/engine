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
#include <game/player.hpp>
#include <game/settings.hpp>

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

        data::NullableRef<renderer::Queue> graphicsQueue_;
        data::NullableRef<renderer::Queue> transferQueue_;
        data::NullableRef<renderer::Queue> presentQueue_;
        data::NullableRef<renderer::Pipeline> basicPipeline_;
        data::NullableRef<renderer::CommandBuffer> transferCommandBuffer_;

        std::vector<renderer::Fence> inFlightFences_;
        std::vector<renderer::Semaphore> acquireSemaphores_;
        std::vector<renderer::Semaphore> presentSemaphores_;
        std::vector<renderer::Framebuffer> framebuffers_;
        std::vector<renderer::CommandBuffer> commandBuffers_;
        std::vector<renderer::Pipeline> pipelines_;
        std::vector<renderer::CommandBuffer> transferCommandBuffers_;

        Counter imageCounter_;
        Counter frameCounter_;

        bool explicitSwapchainRecreate_ = false;

        data::Unique<game::SettingsManager> settingsManager_;
        data::Unique<game::Player> player_;
        data::Unique<game::Camera> camera_;

        game::SettingsConfig settings_;

        std::chrono::time_point<std::chrono::high_resolution_clock> lastFrameTime_;
    };
}