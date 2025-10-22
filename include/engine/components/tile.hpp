#pragma once

#include <renderer/buffer.hpp>

#include <engine/components/transforms.hpp>

#include <glm/glm.hpp>

namespace engine::components {
    struct TileInstance {
        struct alignas(32) Texture {
            struct Sample {
                glm::vec2 position;
                glm::vec2 extent;
            } sample;

            glm::vec2 offset;
            glm::vec2 scale;
        } texture;

        glm::vec2 scale;
        glm::vec3 position;
    };

    struct Tile {
        std::optional<std::uint32_t> group;
    };

    struct TileMesh {
        renderer::Buffer vertexBuffer;
        renderer::Buffer instanceBuffer;
    };
}