#include <world/world.hpp>

#include <filesystem>

world::World world::loadWorld(const std::string& folder, renderer::Device& device, renderer::CommandBuffer& commandBuffer, renderer::Buffer& stagingBuffer, std::size_t& stagingBufferOffset) {
    if (folder.length() == 0 || !std::filesystem::exists(folder)) {
        throw std::runtime_error("Call failed: world::loadWorld(): Filepath to world is invalid");
    }

    bool slashTerminated = folder.back() == '/' || folder.back() == '\\';

    std::string slashTerminator;

    if (!slashTerminated) {
        slashTerminator = "/";
    }

    std::string defaultsPath = folder + slashTerminator + "defaults.json";
    std::string actionsPath = folder + slashTerminator + "actions.json";
    std::string spacesPath = folder + slashTerminator + "spaces.json";
    std::string tilesPath = folder + slashTerminator + "tiles.json";
    std::string triggersPath = folder + slashTerminator + "triggers.json";
    std::string scriptsPath = folder + slashTerminator + "scripts/";

    if (!std::filesystem::exists(defaultsPath)) {
        throw std::runtime_error("Call failed: world::loadWorld(): defaults.json not found in world folder");
    }

    if (!std::filesystem::exists(actionsPath)) {
        throw std::runtime_error("Call failed: world::loadWorld(): actions.json not found in world folder");
    }

    if (!std::filesystem::exists(spacesPath)) {
        throw std::runtime_error("Call failed: world::loadWorld(): spaces.json not found in world folder");
    }

    if (!std::filesystem::exists(tilesPath)) {
        throw std::runtime_error("Call failed: world::loadWorld(): tiles.json not found in world folder");
    }

    if (!std::filesystem::exists(triggersPath)) {
        throw std::runtime_error("Call failed: world::loadWorld(): triggers.json not found in world folder");
    }

    if (!std::filesystem::exists(scriptsPath)) {
        throw std::runtime_error("Call failed: world::loadWorld(): scripts folder not found in world folder");
    }

    World world = {};

    createTileMesh(world.tiles.size(), world.tileMesh, device, stagingBuffer, commandBuffer, stagingBufferOffset);

    return world;
}