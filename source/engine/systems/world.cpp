#include <engine/components/action.hpp>
#include <engine/components/camera.hpp>
#include <engine/components/defaults.hpp>
#include <engine/components/entity_tags.hpp>
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

        nlohmann::json defaultsJson = nlohmann::json::parse(std::string{std::istreambuf_iterator<char>{std::ifstream(defaultsPath).rdbuf()}, {}});
        nlohmann::json actionsJson = nlohmann::json::parse(std::string{std::istreambuf_iterator<char>{std::ifstream(actionsPath).rdbuf()}, {}});
        nlohmann::json spacesJson = nlohmann::json::parse(std::string{std::istreambuf_iterator<char>{std::ifstream(spacesPath).rdbuf()}, {}});
        nlohmann::json tilesJson = nlohmann::json::parse(std::string{std::istreambuf_iterator<char>{std::ifstream(tilesPath).rdbuf()}, {}});
        nlohmann::json triggersJson = nlohmann::json::parse(std::string{std::istreambuf_iterator<char>{std::ifstream(triggersPath).rdbuf()}, {}});

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

        world.defaults = registry.create();
        auto& defaults = registry.emplace<components::Defaults>(world.defaults);

        defaults.name = defaultsJson.at("name").get<std::string>();
        defaults.camera.mode = defaultsCameraMode;
        defaults.camera.position = defaultsCameraPosition;
        defaults.camera.scale = defaultsCameraJson.at("scale").get<float>();

        if (!actionsJson.contains("actions") || !actionsJson.at("actions").is_array()) {
            continue;
        }

        world.actions.reserve(actionsJson.at("actions").size());

        for (auto& actionJson : actionsJson.at("actions")) {
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

            engine.addScript(action.name, scriptsPath + action.script.filepath);
        }

        // --- Load Spaces ---
        if (!spacesJson.contains("spaces") || !spacesJson.at("spaces").is_array()) {
            continue;
        }

        world.spaces.reserve(spacesJson.at("spaces").size());

        for (auto& spaceJson : spacesJson.at("spaces")) {
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
            space.bounds.position = {boundsJson.at("x").get<float>(), boundsJson.at("y").get<float>()};
            space.bounds.extent = {boundsJson.at("width").get<float>(), boundsJson.at("height").get<float>()};

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
                    cameraPositionJson.at("y").get<float>()};
            }
            else {
                continue;
            }

            space.camera.scale = cameraJson.at("scale").get<float>();
        }

        if (!tilesJson.contains("tiles") || !tilesJson.at("tiles").is_array()) {
            continue;
        }

        world.tiles.reserve(tilesJson.at("tiles").size());

        for (auto& tileJson : tilesJson.at("tiles")) {
            if (!tileJson.contains("transform") || !tileJson.at("transform").is_object() ||
                !tileJson.contains("texture") || !tileJson.at("texture").is_object()) {
                continue;
            }

            auto& transformJson = tileJson.at("transform");
            auto& textureJson = tileJson.at("texture");

            world.tiles.emplace_back();
            auto& tile = world.tiles.back();

            if (!transformJson.contains("position") || !transformJson.at("position").is_object() ||
                !transformJson.contains("scale") || !transformJson.at("scale").is_object() ||
                !transformJson.contains("rotation") || !transformJson.at("rotation").is_number_float()) {
                continue;
            }

            auto& positionJson = transformJson.at("position");
            auto& scaleJson = transformJson.at("scale");
            auto& rotationJson = transformJson.at("rotation");

            glm::vec3 position = {0.0f, 0.0f, 0.0f};
            glm::vec2 scale = {1.0f, 1.0f};
            float rotation = 0.0f;

            position.x = positionJson.at("x").get<float>();
            position.y = positionJson.at("y").get<float>();
            position.z = positionJson.at("z").get<float>();

            scale.x = scaleJson.at("width").get<float>();
            scale.y = scaleJson.at("height").get<float>();

            rotation = rotationJson.get<float>();

            float sin = std::sin(glm::radians(rotation));
            float cos = std::cos(glm::radians(rotation));

            tile.transform.position = glm::vec4(position, 1.0f);
            tile.transform.model = glm::mat2{
                glm::vec2{cos * scale.x, -sin * scale.y},
                glm::vec2{sin * scale.x, cos * scale.y},
            };

            if (!textureJson.contains("sample") || !textureJson.at("sample").is_object() ||
                !textureJson.contains("offset") || !textureJson.at("offset").is_object() ||
                !textureJson.contains("scale") || !textureJson.at("scale").is_object()) {
                continue;
            }

            auto& sampleJson = textureJson.at("sample");
            auto& offsetJson = textureJson.at("offset");
            auto& scaleTexJson = textureJson.at("scale");

            tile.texture.sample.position = {
                sampleJson.at("x").get<float>(),
                sampleJson.at("y").get<float>(),
            };
            tile.texture.sample.extent = {
                sampleJson.at("width").get<float>(),
                sampleJson.at("height").get<float>(),
            };
            tile.texture.offset = {
                offsetJson.at("x").get<float>(),
                offsetJson.at("y").get<float>(),
            };
            tile.texture.scale = {
                scaleTexJson.at("x").get<float>(),
                scaleTexJson.at("y").get<float>(),
            };
        }

        if (!triggersJson.contains("triggers") || !triggersJson.at("triggers").is_array()) {
            continue;
        }

        world.triggers.reserve(triggersJson.at("triggers").size());

        for (auto& triggerJson : triggersJson.at("triggers")) {
            if (!triggerJson.contains("bounds") || !triggerJson.at("bounds").is_object()) {
                continue;
            }

            auto& boundsJson = triggerJson.at("bounds");

            if (!boundsJson.contains("x") || !boundsJson.at("x").is_number_float() ||
                !boundsJson.contains("y") || !boundsJson.at("y").is_number_float() ||
                !boundsJson.contains("width") || !boundsJson.at("width").is_number_float() ||
                !boundsJson.contains("height") || !boundsJson.at("height").is_number_float()) {
                continue;
            }

            auto triggerEntity = registry.create();
            world.triggers.push_back(triggerEntity);

            auto& trigger = registry.emplace<components::Trigger>(triggerEntity);

            trigger.bounds.position = {
                boundsJson.at("x").get<float>(),
                boundsJson.at("y").get<float>(),
            };

            trigger.bounds.extent = {
                boundsJson.at("width").get<float>(),
                boundsJson.at("height").get<float>(),
            };

            if (triggerJson.contains("on_enter") && triggerJson.at("on_enter").is_array()) {
                for (auto& eventJson : triggerJson.at("on_enter")) {
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

                    trigger.onEnter.push_back(event);
                }
            }

            if (triggerJson.contains("on_exit") && triggerJson.at("on_exit").is_array()) {
                for (auto& eventJson : triggerJson.at("on_exit")) {
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

                    trigger.onExit.push_back(event);
                }
            }
        }
    }
}

void engine::systems::checkTriggers(entt::registry& registry) {
    for (auto& triggerEntity : registry.view<components::Trigger>()) {
        auto& trigger = registry.get<components::Trigger>(triggerEntity);

        for (auto& activatorEntity : registry.view<components::Position, components::CanTriggerTag>()) {
            auto& position = registry.get<components::Position>(activatorEntity);

            glm::vec2 position2D = {position.position.x, position.position.y};
            glm::vec2 lastPosition2D = {position.lastPosition.x, position.lastPosition.y};
            glm::vec2 scale = {1.0f, 1.0f};

            if (registry.all_of<components::Scale>(activatorEntity)) {
                // scale = registry.get<components::Scale>(activatorEntity).scale;
            }

            auto collides = [&](glm::vec2& pos) {
                glm::vec2 entityMin = pos;
                glm::vec2 entityMax = pos + scale;

                glm::vec2 triggerMin = trigger.bounds.position;
                glm::vec2 triggerMax = trigger.bounds.position + trigger.bounds.extent;

                return (entityMax.x > triggerMin.x) && (entityMin.x < triggerMax.x) &&
                       (entityMax.y > triggerMin.y) && (entityMin.y < triggerMax.y);
            };

            bool lastCollides = collides(lastPosition2D);
            bool thisCollides = collides(position2D);

            trigger.onEnterTriggered = !lastCollides && thisCollides;
            trigger.onExitTriggered = lastCollides && !thisCollides;
        }
    }
}

void engine::systems::performTriggers(entt::registry& registry, Engine& engine) {
    auto runActions = [&](bool& condition, auto& container) {
        if (condition) {
            for (auto& actionEntity : container) {
                auto& action = registry.get<components::Action>(actionEntity.action);

                engine.runFunction(action.name, action.script.function, actionEntity.parameters);
            }
        }

        condition = false;
    };

    for (auto& entity : registry.view<components::Trigger>()) {
        auto& trigger = registry.get<components::Trigger>(entity);

        runActions(trigger.onEnterTriggered, trigger.onEnter);
        runActions(trigger.onExitTriggered, trigger.onExit);
    }
}