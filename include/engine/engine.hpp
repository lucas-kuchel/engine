#pragma once

#include <app/configuration.hpp>
#include <app/context.hpp>
#include <app/window.hpp>

#include <components/action.hpp>
#include <components/camera.hpp>
#include <components/entity_tags.hpp>
#include <components/tile.hpp>
#include <components/transforms.hpp>

#include <engine/input_manager.hpp>
#include <engine/staging_manager.hpp>
#include <engine/tile_mesh.hpp>
#include <engine/tile_pool.hpp>

#include <renderer/renderer.hpp>

#include <entt/entt.hpp>
#include <magic_enum/magic_enum.hpp>
#include <sol/sol.hpp>

#include <chrono>

namespace engine {
    class Engine;

    struct CameraInfo {
        ::components::Camera state;
        float rotation;
        glm::vec2 scale;
        glm::vec2 position;
    };

    template <typename T>
    struct SpanProxy {
        std::span<T> span;

        std::size_t size() const {
            return span.size();
        }

        T& get(std::size_t index) {
            return span[index - 1];
        }
    };

    class EngineAPI {
    public:
        EngineAPI(Engine& engine)
            : engine_(engine) {
        }

        float getActionDuration();
        float getActionTimeElapsed();
        float getDeltaTime();

        CameraInfo getCameraInfo();
        void setCameraInfo(CameraInfo& cameraInfo);

        void bindAction(components::Action& action);
        void addToGroup(const TileProxy& proxy, std::uint32_t group);
        void removeFromGroup(const TileProxy& proxy, std::uint32_t group);
        void setSpace(const std::string& space);
        void resetSpace();

        SpanProxy<TileProxy> getTileGroupProxies(std::uint32_t group);
        SpanProxy<::components::TileInstance> getTileInstances();

    private:
        Engine& engine_;
        components::Action* action_ = nullptr;
    };

    class Engine {
    public:
        Engine();
        ~Engine();

        void run();

        void addScript(const std::string& filepath);
        void runFunction(const std::string& function, std::vector<std::optional<std::string>>& parameters);

        auto& getWorldTilePool() {
            return worldTilePool_;
        }

        auto& getEntityTilePool() {
            return entityTilePool_;
        }

        auto& getUiTilePool() {
            return uiTilePool_;
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

        auto& getAPI() {
            return api_;
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

        auto getCurrentWorld() const {
            return currentWorld_;
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

        EngineAPI api_;
        InputManager inputManager_;

        TilePool worldTilePool_;
        TilePool uiTilePool_;
        TilePool entityTilePool_;

        entt::entity currentCamera_;
        entt::entity currentEntity_;
        entt::entity currentWorld_;

        entt::registry registry_;
        entt::dispatcher dispatcher_;

        sol::state luaState_;

        app::Context context_;
        app::Window window_;

        renderer::Renderer renderer_;

        StagingManager stagingManager_;

        TileMesh worldTileMesh_;
        TileMesh uiTileMesh_;
        TileMesh entityTileMesh_;

        renderer::CommandPool transferCommandPool_;

        renderer::DescriptorSet tilemapDescriptorSet_;

        renderer::DescriptorSetLayout descriptorSetLayout_;

        renderer::DescriptorPool descriptorPool_;

        renderer::Pipeline worldPipeline_;

        renderer::PipelineLayout basicPipelineLayout_;

        renderer::Image tilemapAlbedoImage_;
        renderer::Image tilemapNormalImage_;
        renderer::Image tilemapSpecularImage_;

        renderer::ImageView tilemapAlbedoImageView_;
        renderer::ImageView tilemapNormalImageView_;
        renderer::ImageView tilemapSpecularImageView_;

        renderer::Sampler sampler_;

        renderer::Buffer cameraBuffer_;

        std::vector<renderer::Pipeline> pipelines_;
        std::vector<renderer::DescriptorSet> descriptorSets_;
        std::vector<renderer::CommandBuffer> transferCommandBuffers_;

        using TimePoint = std::chrono::time_point<std::chrono::high_resolution_clock>;

        TimePoint lastFrameTime_;
        TimePoint thisFrameTime_;

        bool running_ = true;
        float deltaTime_ = 0.0f;
    };
}