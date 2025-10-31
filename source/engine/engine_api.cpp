#include <engine/engine.hpp>
#include <engine/engine_api.hpp>

#include <components/space.hpp>
#include <components/world.hpp>

void engine::EngineAPI::addToGroup(const TileProxy& proxy, std::uint32_t group) {
    auto& list = engine_.getWorldTilePool().getProxyGroup(group);

    for (const auto& entry : list) {
        if (entry.index == proxy.index) {
            return;
        }
    }

    list.push_back(proxy);
}

engine::CameraInfo engine::EngineAPI::getCameraInfo() {
    using namespace ::components;

    auto& registry = engine_.getRegistry();
    auto camera = engine_.getCurrentCamera();

    return {
        .state = registry.get<Camera>(camera),
        .rotation = registry.get<Rotation>(camera).angle,
        .scale = registry.get<Scale>(camera).scale,
        .position = registry.get<Position>(camera).position,
    };
}

void engine::EngineAPI::setCameraInfo(CameraInfo& cameraInfo) {
    using namespace ::components;

    auto& registry = engine_.getRegistry();
    auto camera = engine_.getCurrentCamera();

    auto& state = registry.get<Camera>(camera);
    auto& rotation = registry.get<Rotation>(camera).angle;
    auto& scale = registry.get<Scale>(camera).scale;
    auto& position = registry.get<Position>(camera).position;

    state = cameraInfo.state;
    rotation = cameraInfo.rotation;
    scale = cameraInfo.scale;
    position = cameraInfo.position;
}

void engine::EngineAPI::removeFromGroup(const TileProxy& proxy, std::uint32_t group) {
    auto& list = engine_.getWorldTilePool().getProxyGroup(group);

    for (std::size_t i = 0; i < list.size(); i++) {
        if (list[i].index == proxy.index) {
            list[i] = list.back();
            list.pop_back();

            return;
        }
    }
}

void engine::EngineAPI::bindAction(components::Action& action) {
    action_ = &action;
}

float engine::EngineAPI::getActionDuration() {
    return action_->duration;
}

float engine::EngineAPI::getActionTimeElapsed() {
    return action_->elapsed;
}

float engine::EngineAPI::getDeltaTime() {
    return engine_.getDeltaTime();
}

void engine::EngineAPI::setSpace(const std::string& space) {
    using namespace ::components;
    using namespace components;

    auto& registry = engine_.getRegistry();
    auto camera = engine_.getCurrentCamera();
    auto world = engine_.getCurrentWorld();

    auto& worldComponent = registry.get<World>(world);
    auto& cameraComponent = registry.get<Camera>(camera);
    auto& positionComponent = registry.get<Position>(camera);

    for (auto& spaceEntity : worldComponent.spaces) {
        auto& spaceComponent = registry.get<Space>(spaceEntity);

        if (spaceComponent.name != space) {
            continue;
        }

        worldComponent.currentState.camera.mode = spaceComponent.camera.mode;
        worldComponent.currentState.camera.size = spaceComponent.camera.size;

        worldComponent.currentState.physics.kineticFriction = spaceComponent.physics.kineticFriction;
        worldComponent.currentState.physics.staticFriction = spaceComponent.physics.staticFriction;

        // TODO: move to camera update
        cameraComponent.mode = spaceComponent.camera.mode;

        // TODO: add flag to space for camera transitions
        if (true) {
            auto& cameraScaleAnimator = registry.emplace_or_replace<CameraSizeAnimator>(camera);
            cameraScaleAnimator.timeElapsed = 0.0f;
            cameraScaleAnimator.duration = 1.0f;
            cameraScaleAnimator.targetSize = spaceComponent.camera.size;
            cameraScaleAnimator.startSize = cameraComponent.size;

            if (spaceComponent.camera.position.has_value()) {
                auto& cameraPositionAnimator = registry.emplace_or_replace<CameraPositionAnimator>(camera);

                cameraPositionAnimator.timeElapsed = 0.0f;
                cameraPositionAnimator.duration = 1.0f;
                cameraPositionAnimator.startPosition = positionComponent.position;
                cameraPositionAnimator.targetPosition = spaceComponent.camera.position.value();
                worldComponent.currentState.camera.position = spaceComponent.camera.position.value();
            }
        }
        else {
            cameraComponent.size = spaceComponent.camera.size;

            if (spaceComponent.camera.position.has_value()) {
                positionComponent.position = spaceComponent.camera.position.value();
            }
        }

        break;
    }
}

void engine::EngineAPI::resetSpace() {
    using namespace ::components;

    auto& registry = engine_.getRegistry();
    auto camera = engine_.getCurrentCamera();
    auto world = engine_.getCurrentWorld();

    auto& worldComponent = registry.get<World>(world);
    auto& cameraComponent = registry.get<Camera>(camera);
    auto& positionComponent = registry.get<Position>(camera);

    worldComponent.currentState = worldComponent.defaultState;
    cameraComponent.mode = worldComponent.currentState.camera.mode;

    // TODO: add flag to space for camera transitions
    if (true) {
        auto& cameraScaleAnimator = registry.emplace_or_replace<CameraSizeAnimator>(camera);

        cameraScaleAnimator.timeElapsed = 0.0f;
        cameraScaleAnimator.duration = 1.0f;
        cameraScaleAnimator.targetSize = worldComponent.defaultState.camera.size;
        cameraScaleAnimator.startSize = cameraComponent.size;

        if (worldComponent.defaultState.camera.position.has_value()) {
            auto& cameraPositionAnimator = registry.emplace_or_replace<CameraPositionAnimator>(camera);

            cameraPositionAnimator.timeElapsed = 0.0f;
            cameraPositionAnimator.duration = 1.0f;
            cameraPositionAnimator.startPosition = positionComponent.position;
            cameraPositionAnimator.targetPosition = worldComponent.defaultState.camera.position.value();
        }
    }
}

engine::SpanProxy<::components::TileInstance> engine::EngineAPI::getTileInstances() {
    using namespace ::components;

    return SpanProxy<TileInstance>{engine_.getWorldTilePool().data()};
}

engine::SpanProxy<engine::TileProxy> engine::EngineAPI::getTileGroupProxies(std::uint32_t group) {
    using namespace ::components;

    return SpanProxy<TileProxy>{std::span(engine_.getWorldTilePool().getProxyGroup(group))};
}