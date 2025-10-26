#pragma once

#include <renderer/buffer.hpp>

#include <engine/components/transforms.hpp>

#include <glm/glm.hpp>

namespace engine::components {
    struct alignas(16) TileInstance {
        struct Transform {
            glm::vec3 position;
            glm::vec2 scale;
        } transform;

        struct Appearance {
            struct Texture {
                struct Sample {
                    glm::vec2 position;
                    glm::vec2 extent;
                } sample;

                glm::vec2 offset;
                glm::vec2 repeat;
            } texture;

            glm::vec3 colourFactor;
        } appearance;
    };

    struct TileMesh {
        renderer::Buffer vertexBuffer;
        renderer::Buffer instanceBuffer;
    };
}