#pragma once

#include <vector>

#include <vulkanite/window/subsystem.hpp>
#include <vulkanite/window/window.hpp>

#include <engine/input_manager.hpp>
#include <engine/renderer.hpp>
#include <engine/staging_manager.hpp>
#include <engine/tile_mesh.hpp>
#include <engine/tile_pool.hpp>
#include <engine/world_generator.hpp>

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

        auto& getWindow() {
            return window_;
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
        vulkanite::window::WindowCreateInfo createWindow();

        void manageEvents();

        void start();
        void update();
        void render();
        void close();

        void createBasicPipelineResources();

        static std::uint64_t keyIndex(vulkanite::window::Key key);

        void calculateDeltaTime();

        void runPreTransferSystems();
        void runMidTransferSystems();
        void runPostTransferSystems();

        entt::entity currentCamera_;
        entt::entity currentEntity_;
        entt::registry registry_;
        entt::dispatcher dispatcher_;

        vulkanite::window::Subsystem subsystem_;
        vulkanite::window::Window window_;

        engine::Renderer renderer_;

        vulkanite::renderer::Sampler sampler_;
        vulkanite::renderer::Image tilemapAlbedoImage_;
        vulkanite::renderer::Image tilemapNormalImage_;
        vulkanite::renderer::ImageView tilemapAlbedoImageView_;
        vulkanite::renderer::ImageView tilemapNormalImageView_;
        vulkanite::renderer::DescriptorPool descriptorPool_;
        vulkanite::renderer::CommandPool transferCommandPool_;
        vulkanite::renderer::DescriptorSetLayout descriptorSetLayout_;
        vulkanite::renderer::PipelineLayout pipelineLayout_;
        vulkanite::renderer::DescriptorSet tilemapDescriptorSet_;
        vulkanite::renderer::Pipeline worldPipeline_;
        vulkanite::renderer::Buffer cameraBuffer_;

        std::vector<vulkanite::renderer::Pipeline> pipelines_;
        std::vector<vulkanite::renderer::DescriptorSet> descriptorSets_;
        std::vector<vulkanite::renderer::CommandBuffer> transferCommandBuffers_;

        StagingManager stagingManager_;
        TileMesh worldTileMesh_;
        TileMesh entityTileMesh_;
        TimePoint lastFrameTime_;
        TimePoint thisFrameTime_;
        InputManager inputManager_;
        WorldGenerator worldGenerator_;
        TilePool worldTilePool_;
        TilePool entityTilePool_;

        bool running_ = true;
        float deltaTime_ = 0.1f;
    };
}