#pragma once

#include <vulkanite/window/window.hpp>

#include <glm/glm.hpp>

namespace engine {
    struct Settings {
        std::string filepath;

        struct Display {
            glm::uvec2 size;

            vulkanite::window::Visibility mode;
        } display;

        struct Graphics {
            bool vsync;

            std::uint32_t imageCount;
            std::uint32_t renderAheadLimit;
        } graphics;

        static Settings load();
        static void save(const Settings& settings);
    };
}