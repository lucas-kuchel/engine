#pragma once

#include <app/configuration.hpp>
#include <app/context.hpp>
#include <app/window.hpp>
#include <renderer/renderer.hpp>

#include <engine/components/action.hpp>
#include <engine/components/proxy.hpp>
#include <engine/components/tile.hpp>

#include <chrono>

#include <entt/entt.hpp>
#include <sol/sol.hpp>

namespace engine {
    class Engine;

    using TileInstance = components::TileInstance;
    using TileProxy = components::Proxy<components::TileInstance>;

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

        void bindAction(components::Action& action);
        void setSpace(const std::string& space);
        void resetSpace();

        SpanProxy<TileProxy> getTileGroupProxies(std::uint32_t group);
        SpanProxy<TileInstance> getTileInstances();

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

    private:
        app::WindowCreateInfo createWindow();

        void manageEvents();

        void start();
        void update();
        void render();
        void close();

        void createBasicPipelineResources();

        static std::uint64_t keyIndex(app::Key key);

        EngineAPI api_;

        entt::entity currentWorld_;
        entt::entity currentCharacter_;
        entt::entity currentCamera_;

        app::Context context_;
        app::Window window_;

        std::vector<TileInstance> tiles_;
        std::vector<std::vector<TileProxy>> sparseTileGroups_;

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