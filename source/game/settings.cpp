#include <game/settings.hpp>

#include <filesystem>
#include <fstream>

namespace game {
    SettingsManager::SettingsManager(const std::string& path)
        : path_(path) {
        if (!std::filesystem::exists(path_)) {
            throw std::runtime_error(std::format("Construction failed: game::SettingsManager: File does not exist: {}", path_));
        }
    }

    void SettingsManager::load(SettingsConfig& config) {
        std::ifstream file(path_);

        if (!file) {
            throw std::runtime_error(std::format("Call failed: game::SettingsManager::load(): File could not be opened for reading: {}", path_));
        }

        std::string contents(std::istreambuf_iterator<char>(file), {});
        nlohmann::json json = nlohmann::json::parse(contents);

        config.display.size.x = json["display"]["width"].get<std::uint32_t>();
        config.display.size.y = json["display"]["height"].get<std::uint32_t>();
        config.display.resizable = json["display"]["resizable"].get<bool>();

        config.graphics.imageCount = json["graphics"]["imageCount"].get<std::uint32_t>();
        config.graphics.renderAheadLimit = json["graphics"]["renderAheadLimit"].get<std::uint32_t>();
        config.graphics.vsync = json["graphics"]["vsync"].get<bool>();

        config.controls.speed = json["controls"]["speed"].get<float>();

        config.camera.zoom = json["camera"]["zoom"].get<float>();
        config.camera.ease = json["camera"]["ease"].get<float>();

        std::string displayMode = json["display"]["mode"].get<std::string>();

        if (displayMode == "windowed") {
            config.display.mode = app::WindowVisibility::WINDOWED;
        }
        else if (displayMode == "fullscreen") {
            config.display.mode = app::WindowVisibility::FULLSCREEN;
        }
        else {
            throw std::runtime_error(std::format("Call failed: game::SettingsManager::load(): Bad value for setting \"display.mode\": {}", path_));
        }
    }

    void SettingsManager::save(const SettingsConfig& config) {
        nlohmann::json json;

        json["display"]["width"] = config.display.size.x;
        json["display"]["height"] = config.display.size.y;
        json["display"]["resizable"] = config.display.resizable;
        json["display"]["mode"] = (config.display.mode == app::WindowVisibility::FULLSCREEN) ? "fullscreen" : "windowed";

        json["graphics"]["imageCount"] = config.graphics.imageCount;
        json["graphics"]["renderAheadLimit"] = config.graphics.renderAheadLimit;
        json["graphics"]["vsync"] = config.graphics.vsync;

        json["controls"]["speed"] = config.controls.speed;

        json["camera"]["zoom"] = config.camera.zoom;
        json["camera"]["ease"] = config.camera.ease;

        std::ofstream file(path_, std::ios::trunc);

        if (!file) {
            throw std::runtime_error(std::format("Call failed: game::SettingsManager::save(): File could not be opened for writing: {}", path_));
        }

        file << json.dump(4);
    }
}