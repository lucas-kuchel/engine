#pragma once

#include <vulkanite/renderer/renderer.hpp>

#include <engine/tile_pool.hpp>

#include <array>

#include <glm/glm.hpp>

namespace engine {
    class Engine;

    class TileMesh {
    public:
        ~TileMesh();

        void create(Engine& engine);
        void setBaseMesh(const std::array<glm::vec2, 4>& vertices);
        void createInstanceBuffer(std::size_t instanceCount);
        void setInstances(std::span<TileInstance> instances);

        auto getMeshBuffer() const {
            return meshBuffer_;
        }

        auto getInstanceBuffer() const {
            return instanceBuffer_;
        }

    private:
        vulkanite::renderer::Buffer meshBuffer_;
        vulkanite::renderer::Buffer instanceBuffer_;

        Engine* engine_;
    };
}