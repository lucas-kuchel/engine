#pragma once

#include <app/configuration.hpp>
#include <app/context.hpp>
#include <app/window.hpp>
#include <renderer/renderer.hpp>
#include <world/world.hpp>

#include <chrono>

#include <entt/entt.hpp>
#include <sol/sol.hpp>

namespace engine {
    class Engine;

    class EngineAPI {
    public:
        EngineAPI(Engine& engine)
            : engine_(engine) {
        }

        void setSpace(const std::string& space);
        void resetSpace();

    private:
        Engine& engine_;
    };

    class Engine {
    public:
        Engine();
        ~Engine();

        void run();

    private:
        app::WindowCreateInfo createWindow();

        void manageEvents();

        void start();
        void update();
        void render();
        void close();

        void createBasicPipelineResources();

        static std::size_t keyIndex(app::Key key);

        EngineAPI api_;

        app::Context context_;
        app::Window window_;

        renderer::Renderer renderer_;
        renderer::CommandPool transferCommandPool_;
        renderer::DescriptorSet basicDescriptorSet_;
        renderer::DescriptorSetLayout descriptorSetLayout_;
        renderer::DescriptorPool descriptorPool_;
        renderer::Pipeline basicPipeline_;
        renderer::PipelineLayout basicPipelineLayout_;
        renderer::Image tilemapImage_;
        renderer::ImageView tilemapImageView_;
        renderer::Sampler sampler_;

        std::vector<renderer::Buffer> stagingBuffers_;
        std::vector<renderer::Fence> stagingBufferFences_;
        std::vector<renderer::Semaphore> stagingBufferSemaphores_;
        std::vector<renderer::Pipeline> pipelines_;
        std::vector<renderer::DescriptorSet> descriptorSets_;
        std::vector<renderer::CommandBuffer> transferCommandBuffers_;

        bool running_ = true;

        entt::registry registry_;
        entt::dispatcher dispatcher_;

        world::World world_;

        using TimePoint = std::chrono::time_point<std::chrono::high_resolution_clock>;
        using KeyArray = std::array<bool, 93>;

        TimePoint lastFrameTime_;
        TimePoint thisFrameTime_;

        float deltaTime_ = 0.0f;

        KeyArray keysPressed_ = {false};
        KeyArray keysHeld_ = {false};
        KeyArray keysReleased_ = {false};
    };
}