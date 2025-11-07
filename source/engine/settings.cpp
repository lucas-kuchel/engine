#include <engine/settings.hpp>

#include <vulkanite/window/configuration.hpp>

#include <fstream>

#include <nlohmann/json.hpp>

engine::Settings engine::Settings::load() {
    Settings settings;

    std::ifstream file("config/settings.json");

    if (!file) {
        save(settings);
    }

    std::string contents(std::istreambuf_iterator<char>(file), {});
    nlohmann::json json = nlohmann::json::parse(contents);

    settings.display.size.x = json["display"]["width"].get<std::uint32_t>();
    settings.display.size.y = json["display"]["height"].get<std::uint32_t>();

    settings.graphics.imageCount = json["graphics"]["imageCount"].get<std::uint32_t>();
    settings.graphics.renderAheadLimit = json["graphics"]["renderAheadLimit"].get<std::uint32_t>();
    settings.graphics.vsync = json["graphics"]["vsync"].get<bool>();

    std::string displayMode = json["display"]["mode"].get<std::string>();

    if (displayMode == "windowed") {
        settings.display.mode = vulkanite::window::Visibility::WINDOWED;
    }
    else if (displayMode == "fullscreen") {
        settings.display.mode = vulkanite::window::Visibility::FULLSCREEN;
    }
    else {
        throw std::runtime_error("Call failed: engine::Settings::load(): Bad value for setting \"display.mode\"");
    }

    return settings;
}

void engine::Settings::save(const Settings& settings) {
    nlohmann::json json;

    json["display"]["width"] = settings.display.size.x;
    json["display"]["height"] = settings.display.size.y;
    json["display"]["mode"] = (settings.display.mode == vulkanite::window::Visibility::FULLSCREEN) ? "fullscreen" : "windowed";

    json["graphics"]["imageCount"] = settings.graphics.imageCount;
    json["graphics"]["renderAheadLimit"] = settings.graphics.renderAheadLimit;
    json["graphics"]["vsync"] = settings.graphics.vsync;

    std::ofstream file("config/settings.json", std::ios::trunc);

    if (!file) {
        throw std::runtime_error("Call failed: engine::saveSettings(): File could not be opened for writing: config/settings.json");
    }

    file << json.dump(4);
}