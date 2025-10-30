#include <components/action.hpp>
#include <components/camera.hpp>
#include <components/entity_tags.hpp>
#include <components/proxy.hpp>
#include <components/space.hpp>
#include <components/tile.hpp>
#include <components/trigger.hpp>
#include <components/world.hpp>
#include <engine/engine.hpp>
#include <systems/world.hpp>

#include <filesystem>
#include <fstream>

#include <nlohmann/json.hpp>

void systems::loadWorlds(entt::registry& registry, engine::Engine& engine) {
    for (auto& entity : registry.view<components::World>()) {
        auto& world = registry.get<components::World>(entity);

        if (world.defaultState.path.length() == 0 || !std::filesystem::exists(world.defaultState.path)) {
            continue;
        }

        bool slashTerminated = world.defaultState.path.back() == '/' || world.defaultState.path.back() == '\\';

        std::string slashTerminator;

        if (!slashTerminated) {
            slashTerminator = "/";
        }

        std::string defaultsPath = world.defaultState.path + slashTerminator + "world.json";
        std::string actionsPath = world.defaultState.path + slashTerminator + "actions.json";
        std::string spacesPath = world.defaultState.path + slashTerminator + "spaces.json";
        std::string tilesPath = world.defaultState.path + slashTerminator + "tiles.json";
        std::string triggersPath = world.defaultState.path + slashTerminator + "triggers.json";
        std::string collidersPath = world.defaultState.path + slashTerminator + "colliders.json";
        std::string scriptsPath = world.defaultState.path + slashTerminator + "scripts/";

        if (!std::filesystem::exists(defaultsPath)) {
            continue;
        }

        if (!std::filesystem::exists(actionsPath)) {
            continue;
        }

        if (!std::filesystem::exists(spacesPath)) {
            continue;
        }

        if (!std::filesystem::exists(tilesPath)) {
            continue;
        }

        if (!std::filesystem::exists(triggersPath)) {
            continue;
        }

        if (!std::filesystem::exists(scriptsPath)) {
            continue;
        }

        if (!std::filesystem::exists(collidersPath)) {
            continue;
        }

        nlohmann::json defaultsJson = nlohmann::json::parse(std::string{std::istreambuf_iterator<char>{std::ifstream(defaultsPath).rdbuf()}, {}});
        nlohmann::json actionsJson = nlohmann::json::parse(std::string{std::istreambuf_iterator<char>{std::ifstream(actionsPath).rdbuf()}, {}});
        nlohmann::json spacesJson = nlohmann::json::parse(std::string{std::istreambuf_iterator<char>{std::ifstream(spacesPath).rdbuf()}, {}});
        nlohmann::json tilesJson = nlohmann::json::parse(std::string{std::istreambuf_iterator<char>{std::ifstream(tilesPath).rdbuf()}, {}});
        nlohmann::json triggersJson = nlohmann::json::parse(std::string{std::istreambuf_iterator<char>{std::ifstream(triggersPath).rdbuf()}, {}});
        nlohmann::json collidersJson = nlohmann::json::parse(std::string{std::istreambuf_iterator<char>{std::ifstream(collidersPath).rdbuf()}, {}});

        for (const auto& entry : std::filesystem::recursive_directory_iterator(scriptsPath)) {
            if (!entry.is_regular_file() || entry.path().extension() != ".lua") {
                continue;
            }

            engine.addScript(entry.path().string());
        }

        auto& defaultsCameraJson = defaultsJson.at("camera");
        auto& defaultsCameraModeJson = defaultsCameraJson.at("mode");
        auto& defaultsCameraScaleJson = defaultsCameraJson.at("scale");
        auto& defaultsPhysicsJson = defaultsJson.at("physics");

        auto& worldTilePool = engine.getWorldTilePool();

        std::string defaultsCameraMode = defaultsCameraModeJson.get<std::string>();

        if (defaultsCameraMode == "follow") {
            world.defaultState.camera.mode = components::CameraMode::FOLLOW;
        }
        else if (defaultsCameraMode == "locked") {
            auto& defaultsCameraPositionJson = defaultsCameraJson.at("position");

            world.defaultState.camera.mode = components::CameraMode::LOCKED;

            world.defaultState.camera.position = {
                defaultsCameraPositionJson.at("x").get<float>(),
                defaultsCameraPositionJson.at("y").get<float>(),
            };
        }
        else {
            throw std::runtime_error("Map loading failed: default camera mode unrecognised");
        }

        world.defaultState.physics.kineticFriction = defaultsPhysicsJson.at("kinetic_friction").get<float>();
        world.defaultState.physics.staticFriction = defaultsPhysicsJson.at("static_friction").get<float>();
        world.defaultState.name = defaultsJson.at("name").get<std::string>();
        world.defaultState.camera.size = defaultsCameraScaleJson.get<float>();

        world.currentState = world.defaultState;

        world.actions.reserve(actionsJson.size());
        world.spaces.reserve(spacesJson.size());
        world.tiles.reserve(tilesJson.size());
        world.triggers.reserve(triggersJson.size());
        world.colliders.reserve(collidersJson.size());

        for (auto& actionJson : actionsJson) {
            auto& scriptJson = actionJson.at("script");

            auto& actionEntity = world.actions.emplace_back(registry.create());
            auto& action = registry.emplace<components::Action>(actionEntity);

            action.name = actionJson.at("name").get<std::string>();
            action.script.filepath = scriptJson.at("filepath").get<std::string>();
            action.script.function = scriptJson.at("function").get<std::string>();

            action.duration = 0.0f;
            action.elapsed = 0.0f;

            if (actionJson.contains("duration") && actionJson.at("duration").is_number_float()) {
                action.duration = actionJson.at("duration").get<float>();
            }
        }

        for (auto& spaceJson : spacesJson) {
            auto& boundsJson = spaceJson.at("bounds");
            auto& physicsJson = spaceJson.at("physics");
            auto& spaceEntity = world.spaces.emplace_back(registry.create());
            auto& space = registry.emplace<components::Space>(spaceEntity);

            space.name = spaceJson.at("name").get<std::string>();

            space.bounds.position = {
                boundsJson.at("x").get<float>(),
                boundsJson.at("y").get<float>(),
            };

            space.bounds.extent = {
                boundsJson.at("width").get<float>(),
                boundsJson.at("height").get<float>(),
            };

            space.physics.kineticFriction = physicsJson.at("kinetic_friction").get<float>();
            space.physics.staticFriction = physicsJson.at("static_friction").get<float>();

            auto& cameraJson = spaceJson.at("camera");
            std::string cameraMode = cameraJson.at("mode").get<std::string>();

            if (cameraMode == "follow") {
                space.camera.mode = components::CameraMode::FOLLOW;
                space.camera.position = std::nullopt;
            }
            else if (cameraMode == "locked") {
                space.camera.mode = components::CameraMode::LOCKED;

                auto& cameraPositionJson = cameraJson.at("position");

                space.camera.position = glm::vec2{
                    cameraPositionJson.at("x").get<float>(),
                    cameraPositionJson.at("y").get<float>(),
                };
            }
            else {
                throw std::runtime_error("Map loading failed: space camera mode unrecognised");
            }

            space.camera.size = cameraJson.at("scale").get<float>();
        }

        for (auto& tileJson : tilesJson) {
            auto& groupsJson = tileJson.at("groups");
            auto& transformJson = tileJson.at("transform");
            auto& appearanceJson = tileJson.at("appearance");

            auto& textureJson = appearanceJson.at("texture");
            auto& colourFactorJson = appearanceJson.at("colour_factor");

            auto& positionJson = transformJson.at("position");
            auto& scaleJson = transformJson.at("scale");

            auto& sampleJson = textureJson.at("sample");
            auto& samplePositionJson = sampleJson.at("position");
            auto& sampleExtentJson = sampleJson.at("extent");
            auto& offsetJson = textureJson.at("offset");
            auto& repeatJson = textureJson.at("repeat");

            auto proxy = worldTilePool.insert({});
            auto& tileInstance = worldTilePool.get(proxy);

            for (auto& groupJson : groupsJson) {
                std::uint32_t group = groupJson.get<std::uint32_t>();

                auto& groupList = worldTilePool.getProxyGroup(group);

                groupList.push_back(proxy);
            }

            tileInstance.appearance.colourFactor = {
                colourFactorJson.at("r").get<float>(),
                colourFactorJson.at("g").get<float>(),
                colourFactorJson.at("b").get<float>(),
            };

            tileInstance.transform.position = {
                positionJson.at("x").get<float>(),
                positionJson.at("y").get<float>(),
                positionJson.at("z").get<float>(),
            };

            tileInstance.transform.scale = {
                scaleJson.at("width").get<float>(),
                scaleJson.at("height").get<float>(),
            };

            tileInstance.appearance.texture.sample.position = {
                samplePositionJson.at("x").get<float>(),
                samplePositionJson.at("y").get<float>(),
            };

            tileInstance.appearance.texture.sample.extent = {
                sampleExtentJson.at("width").get<float>(),
                sampleExtentJson.at("height").get<float>(),
            };

            tileInstance.appearance.texture.offset = {
                offsetJson.at("x").get<float>(),
                offsetJson.at("y").get<float>(),
            };

            tileInstance.appearance.texture.repeat = {
                repeatJson.at("x").get<float>(),
                repeatJson.at("y").get<float>(),
            };

            auto& tile = world.tiles.emplace_back();

            tile = registry.create();

            registry.emplace<components::Position>(tile, tileInstance.transform.position);
            registry.emplace<components::Scale>(tile, tileInstance.transform.scale);
            registry.emplace<components::TileTag>(tile);
            registry.emplace<components::TileProxy>(tile, proxy);
        }

        for (auto& triggerJson : triggersJson) {
            auto& transformJson = triggerJson.at("transform");
            auto& triggerEntity = world.triggers.emplace_back(registry.create());
            auto& trigger = registry.emplace<components::Trigger>(triggerEntity);
            auto& position = registry.emplace<components::Position>(triggerEntity);
            auto& scale = registry.emplace<components::Scale>(triggerEntity);

            registry.emplace<components::TriggerTag>(triggerEntity);

            position.position = {
                transformJson.at("position").at("x").get<float>(),
                transformJson.at("position").at("y").get<float>(),
            };

            scale.scale = {
                transformJson.at("scale").at("width").get<float>(),
                transformJson.at("scale").at("height").get<float>(),
            };

            auto loadEvents = [&](std::string_view listName, std::vector<components::Trigger::Event>& events) {
                for (auto& eventJson : triggerJson.at(listName)) {
                    components::Trigger::Event& event = events.emplace_back();
                    std::string actionName = eventJson.at("action").get<std::string>();

                    for (auto& action : world.actions) {
                        auto& actionComponent = registry.get<components::Action>(action);
                        if (actionName == actionComponent.name) {
                            event.action = action;
                        }
                    }

                    for (auto& paramJson : eventJson.at("parameters")) {
                        if (paramJson.is_string()) {
                            event.parameters.push_back(paramJson.get<std::string>());
                        }

                        event.parameters.push_back(std::nullopt);
                    }
                }
            };

            loadEvents("on_collide", trigger.onCollide);
            loadEvents("on_separate", trigger.onSeparate);
        }

        for (auto& colliderJson : collidersJson) {
            auto colliderEntity = world.colliders.emplace_back(registry.create());
            auto& position = registry.emplace<components::Position>(colliderEntity);
            auto& scale = registry.emplace<components::Scale>(colliderEntity);

            registry.emplace<components::ColliderTag>(colliderEntity);

            position.position = {
                colliderJson.at("position").at("x").get<float>(),
                colliderJson.at("position").at("y").get<float>(),
            };

            scale.scale = {
                colliderJson.at("scale").at("width").get<float>(),
                colliderJson.at("scale").at("height").get<float>(),
            };
        }
    }
}

void systems::testCollisions(entt::registry& registry) {
    for (auto& colliderEntity : registry.view<components::ColliderTag, components::Scale, components::Position>()) {
        auto& colliderPosition = registry.get<components::Position>(colliderEntity);
        auto& colliderScale = registry.get<components::Scale>(colliderEntity);

        auto resolveAABB = [](components::Position& aPos, components::Velocity& aVel, components::Scale& aScale,
                              const components::Position& bPos, const components::Scale& bScale) {
            glm::vec2 aMin = aPos.position;
            glm::vec2 aMax = aPos.position + aScale.scale;
            glm::vec2 bMin = bPos.position;
            glm::vec2 bMax = bPos.position + bScale.scale;

            if (aMax.x <= bMin.x || aMin.x >= bMax.x ||
                aMax.y <= bMin.y || aMin.y >= bMax.y)
                return;

            float overlapX1 = bMax.x - aMin.x;
            float overlapX2 = aMax.x - bMin.x;
            float overlapY1 = bMax.y - aMin.y;
            float overlapY2 = aMax.y - bMin.y;

            float minX = (overlapX1 < overlapX2) ? overlapX1 : -overlapX2;
            float minY = (overlapY1 < overlapY2) ? overlapY1 : -overlapY2;

            if (std::abs(minX) < std::abs(minY)) {
                aPos.position.x += minX;
                aVel.velocity.x = 0.0f;
            }
            else {
                aPos.position.y += minY;
                aVel.velocity.y = 0.0f;
            }
        };

        for (auto& activatorEntity : registry.view<components::Position, components::Velocity, components::Scale, components::CanTriggerTag>()) {
            auto& activatorPosition = registry.get<components::Position>(activatorEntity);
            auto& activatorVelocity = registry.get<components::Velocity>(activatorEntity);
            auto& activatorScale = registry.get<components::Scale>(activatorEntity);

            resolveAABB(activatorPosition, activatorVelocity, activatorScale, colliderPosition, colliderScale);
        }
    }
}

void systems::checkTriggers(entt::registry& registry) {
    for (auto& triggerEntity : registry.view<components::Trigger, components::TriggerTag, components::Scale, components::Position>()) {
        auto& trigger = registry.get<components::Trigger>(triggerEntity);
        auto& triggerPosition = registry.get<components::Position>(triggerEntity);
        auto& triggerScale = registry.get<components::Scale>(triggerEntity);

        auto collides = [&](glm::vec2& position, glm::vec2& scale) {
            glm::vec2 entityMin = position;
            glm::vec2 entityMax = position + scale;
            glm::vec2 triggerMin = triggerPosition.position;
            glm::vec2 triggerMax = triggerPosition.position + triggerScale.scale;

            return (entityMax.x > triggerMin.x) && (entityMin.x < triggerMax.x) &&
                   (entityMax.y > triggerMin.y) && (entityMin.y < triggerMax.y);
        };

        for (auto& activatorEntity : registry.view<components::Position, components::Scale, components::CanTriggerTag, components::Last<components::Position>>()) {
            auto& activatorPosition = registry.get<components::Position>(activatorEntity);
            auto& activatorLastPosition = registry.get<components::Last<components::Position>>(activatorEntity);
            auto& activatorScale = registry.get<components::Scale>(activatorEntity);

            bool lastCollides = collides(activatorLastPosition.value.position, activatorScale.scale);
            bool thisCollides = collides(activatorPosition.position, activatorScale.scale);

            trigger.collideTriggered = trigger.collideTriggered | (!lastCollides && thisCollides);
            trigger.separateTriggered = trigger.separateTriggered | (lastCollides && !thisCollides);
        }
    }
}

void systems::performTriggers(entt::registry& registry, engine::Engine& engine, float deltaTime) {
    auto runActions = [&](bool& condition, auto& container) {
        for (auto& actionEntity : container) {
            auto& action = registry.get<components::Action>(actionEntity.action);
            if (condition || (action.duration > 0.0f && action.elapsed > 0.0f)) {
                engine.getAPI().bindAction(action);

                if (action.duration > 0.0f) {
                    action.elapsed += deltaTime;
                }

                engine.runFunction(action.script.function, actionEntity.parameters);

                if (action.duration > 0.0f && action.elapsed >= action.duration) {
                    action.elapsed = 0.0f;
                }
            }
        }

        condition = false;
    };

    for (auto& entity : registry.view<components::Trigger>()) {
        auto& trigger = registry.get<components::Trigger>(entity);

        runActions(trigger.collideTriggered, trigger.onCollide);
        runActions(trigger.separateTriggered, trigger.onSeparate);
    }
}