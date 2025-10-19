#pragma once

#include <world/action.hpp>
#include <world/defaults.hpp>
#include <world/space.hpp>
#include <world/tile.hpp>
#include <world/trigger.hpp>

namespace world {
    struct World {
        std::string name;

        Defaults defaults;

        std::vector<Action> actions;
        std::vector<Space> spaces;
        std::vector<Tile> tiles;
        std::vector<Trigger> triggers;

        std::optional<Space> currentSpace;

        TileMesh tileMesh;
    };

    World loadWorld(const std::string& folder, renderer::Device& device, renderer::CommandBuffer& commandBuffer, renderer::Buffer& stagingBuffer, std::size_t& stagingBufferOffset);
}