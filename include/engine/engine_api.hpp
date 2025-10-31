#pragma once

#include <components/action.hpp>
#include <components/camera.hpp>

#include <engine/tile_pool.hpp>

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
}