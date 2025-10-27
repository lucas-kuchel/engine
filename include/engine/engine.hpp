#pragma once

#include <app/configuration.hpp>
#include <app/context.hpp>
#include <app/window.hpp>
#include <renderer/renderer.hpp>

#include <engine/components/action.hpp>
#include <engine/components/camera.hpp>
#include <engine/components/entity_tags.hpp>
#include <engine/components/proxy.hpp>
#include <engine/components/tile.hpp>
#include <engine/components/transforms.hpp>

#include <chrono>

#include <entt/entt.hpp>
#include <sol/sol.hpp>

namespace engine {
    class Engine;

    using TileInstance = components::TileInstance;
    using TileProxy = components::Proxy<components::TileInstance>;
    using ColliderProxy = components::Proxy<components::ColliderTag>;

    struct CameraInfo {
        components::Camera state;
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
        SpanProxy<ColliderProxy> getColliderGroupProxies(std::uint32_t group);
        SpanProxy<TileInstance> getTileInstances();
        SpanProxy<C> getColliderInstances();

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

        auto& getTiles() {
            return tiles_;
        }

        auto& getSparseTileGroups() {
            return sparseTileGroups_;
        }

        auto& getSparseColliderGroups() {
            return sparseColliderGroups_;
        }

        auto& getPlayer() {
            return currentCharacter_;
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

        void getDeltaTime();

        void runPreTransferSystems();
        void runMidTransferSystems();
        void runPostTransferSystems();

        EngineAPI api_;

        entt::entity currentWorld_;
        entt::entity currentCharacter_;
        entt::entity currentCamera_;
        entt::entity pauseButton_;

        app::Context context_;
        app::Window window_;

        std::vector<TileInstance> tiles_;
        std::vector<std::vector<TileProxy>> sparseTileGroups_;
        std::vector<std::vector<ColliderProxy>> sparseColliderGroups_;

        std::size_t worldTileCount_ = 0;
        std::size_t worldTileFirst_ = 0;

        std::size_t buttonsTileCount_ = 0;
        std::size_t buttonsTileFirst_ = 0;

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

        bool running_ = true;

        entt::registry registry_;
        entt::dispatcher dispatcher_;

        glm::vec2 mousePosition_;
        glm::vec2 lastMousePosition_;

        sol::state luaState_;

        using TimePoint = std::chrono::time_point<std::chrono::high_resolution_clock>;
        using KeyArray = std::array<bool, 93>;

        TimePoint lastFrameTime_;
        TimePoint thisFrameTime_;

        float deltaTime_ = 0.0f;

        KeyArray keysPressed_ = {false};
        KeyArray keysHeld_ = {false};
        KeyArray keysReleased_ = {false};

        friend class EngineAPI;
    };
}