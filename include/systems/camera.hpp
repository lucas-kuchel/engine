#pragma once

#include <entt/entt.hpp>

namespace engine {
    class Engine;
}

namespace systems::cameras {
    void calculateCameraData(engine::Engine& engine);
    void animateCameraSizes(engine::Engine& engine);
    void animateCameraPositions(engine::Engine& engine);
    void makeCamerasFollowTarget(engine::Engine& engine);
    void uploadCameraData(engine::Engine& engine);
}