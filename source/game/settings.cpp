#include <game/settings.hpp>

#include <fstream>

#include <nlohmann/json.hpp>

namespace game {
    void loadSettings(Settings& settings) {
        std::ifstream file("config/settings.json");

        if (!file) {
            throw std::runtime_error("Call failed: game::loadSettings(): Failed to open file: config/settings.json");
        }

        std::string contents(std::istreambuf_iterator<char>(file), {});
        nlohmann::json json = nlohmann::json::parse(contents);

        settings.display.size.x = json["display"]["width"].get<std::uint32_t>();
        settings.display.size.y = json["display"]["height"].get<std::uint32_t>();
        settings.display.resizable = json["display"]["resizable"].get<bool>();

        settings.graphics.imageCount = json["graphics"]["imageCount"].get<std::uint32_t>();
        settings.graphics.renderAheadLimit = json["graphics"]["renderAheadLimit"].get<std::uint32_t>();
        settings.graphics.vsync = json["graphics"]["vsync"].get<bool>();

        settings.camera.scale = json["camera"]["scale"].get<float>();
        settings.camera.ease = json["camera"]["ease"].get<float>();

        std::string displayMode = json["display"]["mode"].get<std::string>();

        if (displayMode == "windowed") {
            settings.display.mode = app::WindowVisibility::WINDOWED;
        }
        else if (displayMode == "fullscreen") {
            settings.display.mode = app::WindowVisibility::FULLSCREEN;
        }
        else {
            throw std::runtime_error("Call failed: game::loadSettings(): Bad value for setting \"display.mode\": config/settings.json");
        }
    }

    void saveSettings(const Settings& settings) {
        nlohmann::json json;

        json["display"]["width"] = settings.display.size.x;
        json["display"]["height"] = settings.display.size.y;
        json["display"]["resizable"] = settings.display.resizable;
        json["display"]["mode"] = (settings.display.mode == app::WindowVisibility::FULLSCREEN) ? "fullscreen" : "windowed";

        json["graphics"]["imageCount"] = settings.graphics.imageCount;
        json["graphics"]["renderAheadLimit"] = settings.graphics.renderAheadLimit;
        json["graphics"]["vsync"] = settings.graphics.vsync;

        json["camera"]["scale"] = settings.camera.scale;
        json["camera"]["ease"] = settings.camera.ease;

        std::ofstream file("config/settings.json", std::ios::trunc);

        if (!file) {
            throw std::runtime_error("Call failed: game::saveSettings(): File could not be opened for writing: config/settings.json");
        }

        file << json.dump(4);
    }
}