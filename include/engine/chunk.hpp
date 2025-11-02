#pragma once

#include <vector>

#include <entt/entt.hpp>
#include <glm/glm.hpp>

namespace engine {
    class Engine;

    struct Chunk {
        std::vector<entt::entity> tiles;

        glm::ivec3 position;
    };

    glm::vec2 worldToScreenSpace(glm::vec3 position);
    glm::vec3 screenToWorldSpace(glm::vec2 screen, float y = 0.0f);
    void generateChunk(engine::Chunk& chunk, engine::Engine& engine);
    void sortChunk(engine::Chunk& chunk, engine::Engine& engine);
}