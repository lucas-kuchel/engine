#pragma once

#include <app/configuration.hpp>
#include <app/context.hpp>
#include <app/window.hpp>
#include <components/action.hpp>
#include <components/camera.hpp>
#include <components/entity_tags.hpp>
#include <components/proxy.hpp>
#include <components/tile.hpp>
#include <components/transforms.hpp>
#include <engine/input_manager.hpp>
#include <engine/tile_pool.hpp>
#include <entt/entt.hpp>
#include <magic_enum/magic_enum.hpp>
#include <renderer/renderer.hpp>
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
        void addToGroup(const ::components::Proxy<::components::TileInstance>& proxy, std::uint32_t group);
        void removeFromGroup(const ::components::Proxy<::components::TileInstance>& proxy, std::uint32_t group);
        void setSpace(const std::string& space);
        void resetSpace();

        SpanProxy<::components::Proxy<::components::TileInstance>> getTileGroupProxies(std::uint32_t group);
        SpanProxy<::components::Proxy<::components::ColliderTag>> getColliderGroupProxies(std::uint32_t group);
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

        auto& getCharacterTilePool() {
            return characterTilePool_;
        }

        auto& getUiTilePool() {
            return uiTilePool_;
        }

        auto& getCurrentCharacter() {
            return currentCharacter_;
        }

        auto& getCurrentCamera() {
            return currentCamera_;
        }

        auto& getCurrentWorld() {
            return currentWorld_;
        }

        auto& getRegistry() {
            return registry_;
        }

        auto& getAPI() {
            return api_;
        }

        auto getDeltaTime() const {
            return deltaTime_;
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
        TilePool characterTilePool_;

        entt::registry registry_;
        entt::dispatcher dispatcher_;

        sol::state luaState_;

        app::Context context_;
        app::Window window_;

        renderer::Renderer renderer_;
        renderer::CommandPool transferCommandPool_;
        renderer::DescriptorSet tilemapDescriptorSet_;
        renderer::DescriptorSet buttonsDescriptorSet_;
        renderer::DescriptorSetLayout descriptorSetLayout_;
        renderer::DescriptorPool descriptorPool_;
        renderer::Pipeline worldPipeline_;
        renderer::Pipeline uiPipeline_;
        renderer::PipelineLayout basicPipelineLayout_;

        renderer::Image tilemapImage_;
        renderer::Image buttonsImage_;
        renderer::ImageView tilemapImageView_;
        renderer::ImageView buttonsImageView_;
        renderer::Sampler sampler_;

        std::vector<renderer::Buffer> stagingBuffers_;
        std::vector<renderer::Fence> stagingBufferFences_;
        std::vector<renderer::Semaphore> stagingBufferSemaphores_;
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