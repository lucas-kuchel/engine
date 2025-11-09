#pragma once

#include <cstdint>
#include <string_view>

#include <vulkanite/renderer/renderer.hpp>

namespace engine {
    class Renderer;

    struct TextureHandle {
        std::uint64_t index = 0;
        std::uint32_t generation = 0;
        std::uint32_t uniqueIdentifier = 0;
    };

    class TextureManager {
    public:
        TextureManager(Renderer& renderer);
        ~TextureManager();

        TextureHandle insert(std::string_view filepath);

    private:
        Renderer& renderer_;
    };
}