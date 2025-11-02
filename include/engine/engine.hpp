#pragma once

#include <optional>
#include <string>
#include <vector>

#include <app/configuration.hpp>
#include <app/context.hpp>
#include <app/window.hpp>

#include <engine/input_manager.hpp>
#include <engine/staging_manager.hpp>
#include <engine/tile_mesh.hpp>
#include <engine/tile_pool.hpp>
#include <engine/world_generator.hpp>

#include <renderer/renderer.hpp>

#include <entt/entt.hpp>

#include <chrono>

namespace engine {
    using TimePoint = std::chrono::time_point<std::chrono::high_resolution_clock>;

    class Engine {
    public:
        Engine();

        void run();

        auto& getWorldTilePool() {
            return worldTilePool_;
        }

        auto& getEntityTilePool() {
            return entityTilePool_;
        }

        auto& getInputManager() {
            return inputManager_;
        }

        auto& getRegistry() {
            return registry_;
        }

        auto& getRenderer() {
            return renderer_;
        }

        auto& getStagingManager() {
            return stagingManager_;
        }

        auto& getTransferBuffer() {
            return transferCommandBuffers_[renderer_.getFrameCounter().index];
        }

        auto& getCameraBuffer() {
            return cameraBuffer_;
        }

        auto& getWorldGenerator() {
            return worldGenerator_;
        }

        auto getDeltaTime() const {
            return deltaTime_;
        }

        auto getCurrentCamera() const {
            return currentCamera_;
        }

        auto getCurrentEntity() const {
            return currentEntity_;
        }

    private:
        app::WindowCreateInfo createWindow();

        void manageEvents();

        void start();
        void update();
        void render();
        void close();

        void createBasicPipelineResources();

        static std::uint64_t keyIndex(app::Key key);

        void calculateDeltaTime();

        void runPreTransferSystems();
        void runMidTransferSystems();
        void runPostTransferSystems();

        InputManager inputManager_;
        WorldGenerator worldGenerator_;

        TilePool worldTilePool_;
        TilePool entityTilePool_;

        entt::entity currentCamera_;
        entt::entity currentEntity_;

        entt::registry registry_;
        entt::dispatcher dispatcher_;

        app::Context context_;
        app::Window window_;

        renderer::Renderer renderer_;
        renderer::CommandPool transferCommandPool_;
        renderer::DescriptorSet tilemapDescriptorSet_;
        renderer::DescriptorSetLayout descriptorSetLayout_;
        renderer::DescriptorPool descriptorPool_;
        renderer::Pipeline worldPipeline_;
        renderer::PipelineLayout pipelineLayout_;
        renderer::Image tilemapAlbedoImage_;
        renderer::ImageView tilemapAlbedoImageView_;
        renderer::Sampler sampler_;
        renderer::Buffer cameraBuffer_;

        std::vector<renderer::Pipeline> pipelines_;
        std::vector<renderer::DescriptorSet> descriptorSets_;
        std::vector<renderer::CommandBuffer> transferCommandBuffers_;

        StagingManager stagingManager_;

        TileMesh worldTileMesh_;
        TileMesh entityTileMesh_;

        TimePoint lastFrameTime_;
        TimePoint thisFrameTime_;

        bool running_ = true;
        float deltaTime_ = 0.0f;
    };
}