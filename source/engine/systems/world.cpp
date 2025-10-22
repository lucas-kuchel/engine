#include <engine/components/action.hpp>
#include <engine/components/camera.hpp>
#include <engine/components/defaults.hpp>
#include <engine/components/entity_tags.hpp>
#include <engine/components/proxy.hpp>
#include <engine/components/space.hpp>
#include <engine/components/tile.hpp>
#include <engine/components/trigger.hpp>
#include <engine/components/world.hpp>
#include <engine/engine.hpp>
#include <engine/systems/world.hpp>

#include <filesystem>
#include <fstream>

#include <nlohmann/json.hpp>

void engine::systems::loadWorlds(entt::registry& registry, Engine& engine) {
    for (auto& entity : registry.view<components::World>()) {
        auto& world = registry.get<components::World>(entity);

        if (world.path.length() == 0 || !std::filesystem::exists(world.path)) {
            continue;
        }

        bool slashTerminated = world.path.back() == '/' || world.path.back() == '\\';

        std::string slashTerminator;

        if (!slashTerminated) {
            slashTerminator = "/";
        }

        std::string defaultsPath = world.path + slashTerminator + "defaults.json";
        std::string actionsPath = world.path + slashTerminator + "actions.json";
        std::string spacesPath = world.path + slashTerminator + "spaces.json";
        std::string tilesPath = world.path + slashTerminator + "tiles.json";
        std::string triggersPath = world.path + slashTerminator + "triggers.json";
        std::string collidersPath = world.path + slashTerminator + "colliders.json";
        std::string scriptsPath = world.path + slashTerminator + "scripts/";

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
            if (!entry.is_regular_file()) {
                continue;
            }

            if (entry.path().extension() == ".lua") {
                engine.addScript(entry.path().string());
            }
        }

        if (!defaultsJson.contains("name") || !defaultsJson.at("name").is_string() || !defaultsJson.contains("camera") || !defaultsJson.at("camera").is_object()) {
            continue;
        }

        auto& defaultsCameraJson = defaultsJson.at("camera");

        if (!defaultsCameraJson.contains("mode") || !defaultsCameraJson.at("mode").is_string() || !defaultsCameraJson.contains("scale") || !defaultsCameraJson.at("scale").is_number_float()) {
            continue;
        }

        std::string defaultsCameraModeStr = defaultsCameraJson.at("mode").get<std::string>();
        std::optional<glm::vec3> defaultsCameraPosition;
        components::CameraMode defaultsCameraMode;

        if (defaultsCameraModeStr == "follow") {
            defaultsCameraMode = components::CameraMode::FOLLOW;
        }
        else if (defaultsCameraModeStr == "locked") {
            defaultsCameraMode = components::CameraMode::LOCKED;

            if (!defaultsCameraJson.contains("position") || !defaultsCameraJson.at("position").is_object()) {
                continue;
            }

            auto& defaultsCameraPositionJson = defaultsCameraJson.at("position");

            if (!defaultsCameraPositionJson.contains("x") || !defaultsCameraPositionJson.at("x").is_number_float() || !defaultsCameraPositionJson.contains("y") || !defaultsCameraPositionJson.at("y").is_number_float()) {
                continue;
            }

            glm::vec3 position;

            position.x = defaultsCameraPositionJson.at("x").get<float>();
            position.y = defaultsCameraPositionJson.at("y").get<float>();

            defaultsCameraPosition = position;
        }
        else {
            continue;
        }

        world.defaults.name = defaultsJson.at("name").get<std::string>();
        world.defaults.camera.mode = defaultsCameraMode;
        world.defaults.camera.position = defaultsCameraPosition;
        world.defaults.camera.scale = defaultsCameraJson.at("scale").get<float>();

        if (!actionsJson.is_array()) {
            continue;
        }

        world.actions.reserve(actionsJson.size());

        for (auto& actionJson : actionsJson) {
            if (!actionJson.contains("name") || !actionJson.at("name").is_string()) {
                continue;
            }

            if (!actionJson.contains("script") || !actionJson.at("script").is_object()) {
                continue;
            }

            auto& scriptJson = actionJson.at("script");

            if (!scriptJson.contains("filepath") || !scriptJson.at("filepath").is_string()) {
                continue;
            }

            if (!scriptJson.contains("function") || !scriptJson.at("function").is_string()) {
                continue;
            }

            auto actionEntity = registry.create();

            world.actions.push_back(actionEntity);

            auto& action = registry.emplace<components::Action>(actionEntity);

            action.name = actionJson.at("name").get<std::string>();
            action.script.filepath = scriptJson.at("filepath").get<std::string>();
            action.script.function = scriptJson.at("function").get<std::string>();
        }

        if (!spacesJson.is_array()) {
            continue;
        }

        world.spaces.reserve(spacesJson.size());

        for (auto& spaceJson : spacesJson) {
            if (!spaceJson.contains("name") || !spaceJson.at("name").is_string()) {
                continue;
            }

            if (!spaceJson.contains("bounds") || !spaceJson.at("bounds").is_object()) {
                continue;
            }

            auto& boundsJson = spaceJson.at("bounds");

            if (!boundsJson.contains("x") || !boundsJson.at("x").is_number_float() ||
                !boundsJson.contains("y") || !boundsJson.at("y").is_number_float() ||
                !boundsJson.contains("width") || !boundsJson.at("width").is_number_float() ||
                !boundsJson.contains("height") || !boundsJson.at("height").is_number_float()) {
                continue;
            }

            auto spaceEntity = registry.create();
            world.spaces.push_back(spaceEntity);

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

            if (!spaceJson.contains("camera") || !spaceJson.at("camera").is_object()) {
                continue;
            }

            auto& cameraJson = spaceJson.at("camera");

            if (!cameraJson.contains("mode") || !cameraJson.at("mode").is_string() ||
                !cameraJson.contains("scale") || !cameraJson.at("scale").is_number_float()) {
                continue;
            }

            std::string cameraModeStr = cameraJson.at("mode").get<std::string>();

            if (cameraModeStr == "follow") {
                space.camera.mode = components::CameraMode::FOLLOW;
                space.camera.position = std::nullopt;
            }
            else if (cameraModeStr == "locked") {
                space.camera.mode = components::CameraMode::LOCKED;

                if (!cameraJson.contains("position") || !cameraJson.at("position").is_object()) {
                    continue;
                }

                auto& cameraPositionJson = cameraJson.at("position");

                if (!cameraPositionJson.contains("x") || !cameraPositionJson.at("x").is_number_float() ||
                    !cameraPositionJson.contains("y") || !cameraPositionJson.at("y").is_number_float()) {
                    continue;
                }

                space.camera.position = glm::vec2{
                    cameraPositionJson.at("x").get<float>(),
                    cameraPositionJson.at("y").get<float>(),
                };
            }
            else {
                continue;
            }

            space.camera.scale = cameraJson.at("scale").get<float>();
        }

        if (!tilesJson.is_array()) {
            continue;
        }

        world.tiles.reserve(tilesJson.size());

        for (auto& tileJson : tilesJson) {
            if (!tileJson.contains("transform") || !tileJson.at("transform").is_object() ||
                !tileJson.contains("texture") || !tileJson.at("texture").is_object()) {
                continue;
            }

            auto& transformJson = tileJson.at("transform");
            auto& textureJson = tileJson.at("texture");

            auto& tile = world.tiles.emplace_back();
            auto tileIndex = engine.getTiles().size();
            auto& tileInstance = engine.getTiles().emplace_back();

            tile = registry.create();

            auto& tileComponent = registry.emplace<components::Tile>(tile);
            auto& tileProxy = registry.emplace<components::Proxy<components::TileInstance>>(tile, tileIndex);

            registry.emplace<components::StaticTileTag>(tile);

            if (tileJson.contains("group") && tileJson.at("group").is_number_unsigned()) {
                tileComponent.group = tileJson.at("group").get<std::uint32_t>();

                auto& sparseTileGroups = engine.getSparseTileGroups();

                if (sparseTileGroups.size() <= tileComponent.group.value()) {
                    sparseTileGroups.resize(tileComponent.group.value() + 1);
                }

                sparseTileGroups[tileComponent.group.value()].push_back(tileProxy);
            }

            if (!transformJson.contains("position") || !transformJson.at("position").is_object() ||
                !transformJson.contains("scale") || !transformJson.at("scale").is_object()) {
                continue;
            }

            auto& positionJson = transformJson.at("position");
            auto& scaleJson = transformJson.at("scale");

            glm::vec3 position = {0.0f, 0.0f, 0.0f};
            glm::vec2 scale = {1.0f, 1.0f};

            position.x = positionJson.at("x").get<float>();
            position.y = positionJson.at("y").get<float>();
            position.z = positionJson.at("z").get<float>();

            scale.x = scaleJson.at("width").get<float>();
            scale.y = scaleJson.at("height").get<float>();

            tileInstance.position = position;
            tileInstance.scale = scale;

            if (!textureJson.contains("sample") || !textureJson.at("sample").is_object() ||
                !textureJson.contains("offset") || !textureJson.at("offset").is_object() ||
                !textureJson.contains("scale") || !textureJson.at("scale").is_object()) {
                continue;
            }

            auto& sampleJson = textureJson.at("sample");
            auto& offsetJson = textureJson.at("offset");
            auto& scaleTexJson = textureJson.at("scale");

            tileInstance.texture.sample.position = {
                sampleJson.at("x").get<float>(),
                sampleJson.at("y").get<float>(),
            };

            tileInstance.texture.sample.extent = {
                sampleJson.at("width").get<float>(),
                sampleJson.at("height").get<float>(),
            };

            tileInstance.texture.offset = {
                offsetJson.at("x").get<float>(),
                offsetJson.at("y").get<float>(),
            };

            tileInstance.texture.scale = {
                scaleTexJson.at("width").get<float>(),
                scaleTexJson.at("height").get<float>(),
            };
        }

        if (!triggersJson.is_array()) {
            continue;
        }

        world.triggers.reserve(triggersJson.size());

        for (auto& triggerJson : triggersJson) {
            if (!triggerJson.contains("transform") || !triggerJson.at("transform").is_object()) {
                continue;
            }

            auto& transformJson = triggerJson.at("transform");

            if (!transformJson.contains("position") || !transformJson.at("position").is_object() ||
                !transformJson.contains("scale") || !transformJson.at("scale").is_object()) {
                continue;
            }

            auto triggerEntity = registry.create();
            world.triggers.push_back(triggerEntity);

            registry.emplace<components::TriggerTag>(triggerEntity);

            auto& trigger = registry.emplace<components::Trigger>(triggerEntity);
            auto& position = registry.emplace<components::Position>(triggerEntity);
            auto& scale = registry.emplace<components::Scale>(triggerEntity);

            position.position = {
                transformJson.at("position").at("x").get<float>(),
                transformJson.at("position").at("y").get<float>(),
            };

            position.depth = -0.5f;

            scale.scale = {
                transformJson.at("scale").at("width").get<float>(),
                transformJson.at("scale").at("height").get<float>(),
            };

            if (!triggerJson.contains("on_collide") || !triggerJson.at("on_collide").is_array()) {
                continue;
            }

            for (auto& eventJson : triggerJson.at("on_collide")) {
                if (!eventJson.contains("action") || !eventJson.at("action").is_string() ||
                    !eventJson.contains("parameters") || !eventJson.at("parameters").is_array()) {
                    continue;
                }

                components::Trigger::Event event;
                std::string actionName = eventJson.at("action").get<std::string>();

                for (auto& action : world.actions) {
                    auto& actionComponent = registry.get<components::Action>(action);
                    if (actionName == actionComponent.name) {
                        event.action = action;
                    }
                }

                if (event.action == entt::null) {
                    continue;
                }

                for (auto& paramJson : eventJson.at("parameters")) {
                    if (paramJson.is_null()) {
                        event.parameters.push_back(std::nullopt);
                    }
                    else if (paramJson.is_string()) {
                        event.parameters.push_back(paramJson.get<std::string>());
                    }
                    else {
                        event.parameters.push_back(std::nullopt);
                    }
                }

                trigger.onCollide.push_back(event);
            }

            if (!triggerJson.contains("on_separate") || !triggerJson.at("on_separate").is_array()) {
                continue;
            }

            for (auto& eventJson : triggerJson.at("on_separate")) {
                if (!eventJson.contains("action") || !eventJson.at("action").is_string() ||
                    !eventJson.contains("parameters") || !eventJson.at("parameters").is_array()) {
                    continue;
                }

                components::Trigger::Event event;
                std::string actionName = eventJson.at("action").get<std::string>();

                for (auto& action : world.actions) {
                    auto& actionComponent = registry.get<components::Action>(action);
                    if (actionName == actionComponent.name) {
                        event.action = action;
                    }
                }

                if (event.action == entt::null) {
                    continue;
                }

                for (auto& paramJson : eventJson.at("parameters")) {
                    if (paramJson.is_null()) {
                        event.parameters.push_back(std::nullopt);
                    }
                    else if (paramJson.is_string()) {
                        event.parameters.push_back(paramJson.get<std::string>());
                    }
                    else {
                        event.parameters.push_back(std::nullopt);
                    }
                }

                trigger.onSeparate.push_back(event);
            }
        }

        if (!collidersJson.is_array()) {
            continue;
        }

        world.colliders.reserve(collidersJson.size());

        for (auto& colliderJson : collidersJson) {
            if (!colliderJson.contains("position") || !colliderJson.at("position").is_object() ||
                !colliderJson.contains("scale") || !colliderJson.at("scale").is_object()) {
                continue;
            }

            auto triggerEntity = registry.create();
            world.triggers.push_back(triggerEntity);

            registry.emplace<components::ColliderTag>(triggerEntity);

            auto& position = registry.emplace<components::Position>(triggerEntity);
            auto& scale = registry.emplace<components::Scale>(triggerEntity);

            position.position = {
                colliderJson.at("position").at("x").get<float>(),
                colliderJson.at("position").at("y").get<float>(),
            };

            position.depth = -0.5f;

            scale.scale = {
                colliderJson.at("scale").at("width").get<float>(),
                colliderJson.at("scale").at("height").get<float>(),
            };
        }
    }
}

void engine::systems::testCollisions(entt::registry& registry) {
    for (auto& colliderEntity : registry.view<components::ColliderTag, components::Scale, components::Position>()) {
        auto& colliderPosition = registry.get<components::Position>(colliderEntity);
        auto& colliderScale = registry.get<components::Scale>(colliderEntity);

        auto resolveCollision = [&](glm::vec2& position, glm::vec2& velocity, glm::vec2& scale) {
            glm::vec2 entityMin = position;
            glm::vec2 entityMax = position + scale;
            glm::vec2 colliderMin = colliderPosition.position;
            glm::vec2 colliderMax = colliderPosition.position + colliderScale.scale;

            bool overlapX = entityMax.x > colliderMin.x && entityMin.x < colliderMax.x;
            bool overlapY = entityMax.y > colliderMin.y && entityMin.y < colliderMax.y;

            if (!(overlapX && overlapY)) {
                return;
            }

            float overlapLeft = entityMax.x - colliderMin.x;
            float overlapRight = colliderMax.x - entityMin.x;
            float overlapTop = entityMax.y - colliderMin.y;
            float overlapBottom = colliderMax.y - entityMin.y;

            float minOverlapX = (overlapLeft < overlapRight) ? -overlapLeft : overlapRight;
            float minOverlapY = (overlapTop < overlapBottom) ? -overlapTop : overlapBottom;

            if (std::abs(minOverlapX) < std::abs(minOverlapY)) {
                position.x += minOverlapX;
                velocity.x = 0.0f;
            }
            else {
                position.y += minOverlapY;
                velocity.y = 0.0f;
            }
        };

        for (auto& activatorEntity : registry.view<components::Position, components::Velocity, components::Scale, components::CanTriggerTag>()) {
            auto& activatorPosition = registry.get<components::Position>(activatorEntity);
            auto& activatorVelocity = registry.get<components::Velocity>(activatorEntity);
            auto& activatorScale = registry.get<components::Scale>(activatorEntity);

            resolveCollision(activatorPosition.position, activatorVelocity.velocity, activatorScale.scale);
        }
    }
}

void engine::systems::checkTriggers(entt::registry& registry) {
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

        for (auto& activatorEntity : registry.view<components::Position, components::Scale, components::CanTriggerTag>()) {
            auto& activatorPosition = registry.get<components::Position>(activatorEntity);
            auto& activatorScale = registry.get<components::Scale>(activatorEntity);

            bool lastCollides = collides(activatorPosition.lastPosition, activatorScale.scale);
            bool thisCollides = collides(activatorPosition.position, activatorScale.scale);

            trigger.collideTriggered = trigger.collideTriggered | (!lastCollides && thisCollides);
            trigger.separateTriggered = trigger.separateTriggered | (lastCollides && !thisCollides);
        }
    }
}

void engine::systems::performTriggers(entt::registry& registry, Engine& engine) {
    auto runActions = [&](bool& condition, auto& container) {
        if (condition) {
            for (auto& actionEntity : container) {
                auto& action = registry.get<components::Action>(actionEntity.action);

                engine.runFunction(action.script.function, actionEntity.parameters);
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