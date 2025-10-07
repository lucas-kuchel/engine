#pragma once

#include <app/window.hpp>

#include <glm/glm.hpp>

namespace game {
    struct Settings {
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
            float scale;
            float ease;
        } camera;
    };

    void loadSettings(Settings& settings);
    void saveSettings(const Settings& settings);
}