#pragma once

#include <app/window.hpp>

#include <glm/glm.hpp>

#include <string>

#include <nlohmann/json.hpp>

namespace game {
    struct SettingsConfig {
        struct Display {
            glm::uvec2 size;

            app::WindowVisibility mode;

            bool resizable;

        } display;

        struct Graphics {
            bool vsync;

            std::uint32_t imageCount;
            std::uint32_t renderAheadLimit;
        } graphics;

        struct Controls {
            float speed;
        } controls;

        struct Camera {
            float zoom;
            float ease;
        } camera;
    };

    class SettingsManager {
    public:
        SettingsManager(const std::string& path);
        ~SettingsManager() = default;

        void load(SettingsConfig& config);
        void save(const SettingsConfig& config);

    private:
        std::string path_;
    };
}