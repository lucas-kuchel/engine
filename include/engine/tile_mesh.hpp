#pragma once

#include <renderer/buffer.hpp>

#include <components/tile.hpp>

#include <array>

#include <glm/glm.hpp>

namespace engine {
    class Engine;

    class TileMesh {
    public:
        TileMesh(Engine& engine);
        ~TileMesh();

        void setBaseMesh(const std::array<glm::vec2, 4>& vertices);

        void createInstanceBuffer(std::size_t instanceCount);

        void setInstances(std::span<components::TileInstance> instances);

        auto getMeshBuffer() const {
            return meshBuffer_;
        }

        auto getInstanceBuffer() const {
            return instanceBuffer_;
        }

    private:
        renderer::Buffer meshBuffer_;
        renderer::Buffer instanceBuffer_;

        Engine& engine_;
    };
}