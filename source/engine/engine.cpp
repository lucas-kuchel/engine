#include <components/buttons.hpp>
#include <components/camera.hpp>
#include <components/controllers.hpp>
#include <components/entity_tags.hpp>
#include <components/proxy.hpp>
#include <components/space.hpp>
#include <components/tile.hpp>
#include <components/transforms.hpp>
#include <components/world.hpp>
#include <engine/engine.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <systems/buttons.hpp>
#include <systems/camera.hpp>
#include <systems/entities.hpp>
#include <systems/transforms.hpp>
#include <systems/world.hpp>

#include <fstream>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

engine::Engine::Engine()
    : api_(*this), stagingManager_(*this), window_(createWindow()), renderer_(window_), worldTileMesh_(*this), uiTileMesh_(*this), entityTileMesh_(*this) {
    using namespace ::components;

    luaState_.open_libraries(
        sol::lib::base,
        sol::lib::package,
        sol::lib::table,
        sol::lib::string,
        sol::lib::math,
        sol::lib::os,
        sol::lib::debug);

    luaState_.new_usertype<EngineAPI>(
        "EngineAPI",
        "resetSpace", &EngineAPI::resetSpace,
        "setSpace", &EngineAPI::setSpace,
        "getTileGroupProxies", &EngineAPI::getTileGroupProxies,
        "getDeltaTime", &EngineAPI::getDeltaTime,
        "getActionTimeElapsed", &EngineAPI::getActionTimeElapsed,
        "getActionDuration", &EngineAPI::getActionDuration,
        "addToGroup", &EngineAPI::addToGroup,
        "removeFromGroup", &EngineAPI::removeFromGroup,
        "getCameraInfo", &EngineAPI::getCameraInfo,
        "setCameraInfo", &EngineAPI::setCameraInfo,
        "getTileInstances", &EngineAPI::getTileInstances);

    luaState_["engine"] = &api_;

    luaState_.new_usertype<Proxy<TileInstance>>(
        "TileProxy",
        "index", &Proxy<TileInstance>::index);

    luaState_.new_usertype<TileInstance::Appearance::Texture::Sample>(
        "TileInstanceAppearanceTextureSample",
        "position", &TileInstance::Appearance::Texture::Sample::position,
        "extent", &TileInstance::Appearance::Texture::Sample::extent);

    luaState_.new_usertype<TileInstance::Appearance::Texture>(
        "TileInstanceAppearanceTexture",
        "sample", &TileInstance::Appearance::Texture::sample,
        "offset", &TileInstance::Appearance::Texture::offset,
        "repeats", &TileInstance::Appearance::Texture::repeat);

    luaState_.new_usertype<TileInstance::Appearance>(
        "TileInstanceAppearance",
        "texture", &TileInstance::Appearance::texture,
        "colourFactor", &TileInstance::Appearance::colourFactor);

    luaState_.new_usertype<TileInstance::Transform>(
        "TileInstanceTransform",
        "position", &TileInstance::Transform::position,
        "scale", &TileInstance::Transform::scale);

    luaState_.new_usertype<TileInstance>(
        "TileInstance",
        "transform", &TileInstance::transform,
        "appearance", &TileInstance::appearance);

    luaState_.new_usertype<CameraInfo>(
        "CameraInfo",
        "state", &CameraInfo::state,
        "position", &CameraInfo::position,
        "rotation", &CameraInfo::rotation,
        "scale", &CameraInfo::scale);

    luaState_.new_usertype<Camera>(
        "CameraState",
        "near", &Camera::near,
        "far", &Camera::far,
        "size", &Camera::size,
        "mode", &Camera::mode);

    luaState_.new_enum("CameraMode",
                       "FOLLOW", CameraMode::FOLLOW,
                       "LOCKED", CameraMode::LOCKED);

    luaState_.new_usertype<SpanProxy<TileInstance>>(
        "TileInstanceSpan",
        sol::meta_function::length, &SpanProxy<TileInstance>::size,
        sol::meta_function::index, &SpanProxy<TileInstance>::get,
        "__len", &SpanProxy<TileInstance>::size,
        "get", &SpanProxy<TileInstance>::get);

    luaState_.new_usertype<SpanProxy<Proxy<TileInstance>>>(
        "TileProxySpan",
        sol::meta_function::length, &SpanProxy<Proxy<TileInstance>>::size,
        sol::meta_function::index, &SpanProxy<Proxy<TileInstance>>::get,
        "__len", &SpanProxy<Proxy<TileInstance>>::size,
        "get", &SpanProxy<Proxy<TileInstance>>::get);

    luaState_.new_usertype<glm::vec2>(
        "vec2",
        "x", &glm::vec2::x,
        "y", &glm::vec2::y);

    luaState_.new_usertype<glm::vec3>(
        "vec3",
        "x", &glm::vec3::x,
        "y", &glm::vec3::y,
        "z", &glm::vec3::z,
        "r", &glm::vec3::r,
        "g", &glm::vec3::g,
        "b", &glm::vec3::b);

    luaState_.new_usertype<glm::vec4>(
        "vec4",
        "x", &glm::vec4::x,
        "y", &glm::vec4::y,
        "z", &glm::vec4::z,
        "w", &glm::vec4::w,
        "r", &glm::vec4::r,
        "g", &glm::vec4::g,
        "b", &glm::vec4::b,
        "a", &glm::vec4::a);
}

engine::Engine::~Engine() {
}

void engine::EngineAPI::addToGroup(const ::components::Proxy<::components::TileInstance>& proxy, std::uint32_t group) {
    auto& list = engine_.getWorldTilePool().getProxyGroup(group);

    for (const auto& entry : list) {
        if (entry.index == proxy.index) {
            return;
        }
    }

    list.push_back(proxy);
}

engine::CameraInfo engine::EngineAPI::getCameraInfo() {
    using namespace ::components;

    auto& registry = engine_.getRegistry();
    auto camera = engine_.getCurrentCamera();

    return {
        .state = registry.get<Camera>(camera),
        .rotation = registry.get<Rotation>(camera).angle,
        .scale = registry.get<Scale>(camera).scale,
        .position = registry.get<Position>(camera).position,
    };
}

void engine::EngineAPI::setCameraInfo(CameraInfo& cameraInfo) {
    using namespace ::components;

    auto& registry = engine_.getRegistry();
    auto camera = engine_.getCurrentCamera();

    auto& state = registry.get<Camera>(camera);
    auto& rotation = registry.get<Rotation>(camera).angle;
    auto& scale = registry.get<Scale>(camera).scale;
    auto& position = registry.get<Position>(camera).position;

    state = cameraInfo.state;
    rotation = cameraInfo.rotation;
    scale = cameraInfo.scale;
    position = cameraInfo.position;
}

void engine::EngineAPI::removeFromGroup(const ::components::Proxy<::components::TileInstance>& proxy, std::uint32_t group) {
    auto& list = engine_.getWorldTilePool().getProxyGroup(group);

    for (std::size_t i = 0; i < list.size(); i++) {
        if (list[i].index == proxy.index) {
            list[i] = list.back();
            list.pop_back();

            return;
        }
    }
}

void engine::EngineAPI::bindAction(components::Action& action) {
    action_ = &action;
}

float engine::EngineAPI::getActionDuration() {
    return action_->duration;
}

float engine::EngineAPI::getActionTimeElapsed() {
    return action_->elapsed;
}

float engine::EngineAPI::getDeltaTime() {
    return engine_.getDeltaTime();
}

void engine::EngineAPI::setSpace(const std::string& space) {
    using namespace ::components;
    using namespace components;

    auto& registry = engine_.getRegistry();
    auto camera = engine_.getCurrentCamera();
    auto world = engine_.getCurrentWorld();

    auto& worldComponent = registry.get<World>(world);
    auto& cameraComponent = registry.get<Camera>(camera);
    auto& positionComponent = registry.get<Position>(camera);

    for (auto& spaceEntity : worldComponent.spaces) {
        auto& spaceComponent = registry.get<Space>(spaceEntity);

        if (spaceComponent.name != space) {
            continue;
        }

        worldComponent.currentState.camera.mode = spaceComponent.camera.mode;
        worldComponent.currentState.camera.size = spaceComponent.camera.size;

        worldComponent.currentState.physics.kineticFriction = spaceComponent.physics.kineticFriction;
        worldComponent.currentState.physics.staticFriction = spaceComponent.physics.staticFriction;

        // TODO: move to camera update
        cameraComponent.mode = spaceComponent.camera.mode;

        // TODO: add flag to space for camera transitions
        if (true) {
            auto& cameraScaleAnimator = registry.emplace_or_replace<CameraSizeAnimator>(camera);
            cameraScaleAnimator.timeElapsed = 0.0f;
            cameraScaleAnimator.duration = 1.0f;
            cameraScaleAnimator.targetSize = spaceComponent.camera.size;
            cameraScaleAnimator.startSize = cameraComponent.size;

            if (spaceComponent.camera.position.has_value()) {
                auto& cameraPositionAnimator = registry.emplace_or_replace<CameraPositionAnimator>(camera);

                cameraPositionAnimator.timeElapsed = 0.0f;
                cameraPositionAnimator.duration = 1.0f;
                cameraPositionAnimator.startPosition = positionComponent.position;
                cameraPositionAnimator.targetPosition = spaceComponent.camera.position.value();
                worldComponent.currentState.camera.position = spaceComponent.camera.position.value();
            }
        }
        else {
            cameraComponent.size = spaceComponent.camera.size;

            if (spaceComponent.camera.position.has_value()) {
                positionComponent.position = spaceComponent.camera.position.value();
            }
        }

        break;
    }
}

void engine::EngineAPI::resetSpace() {
    using namespace ::components;

    auto& registry = engine_.getRegistry();
    auto camera = engine_.getCurrentCamera();
    auto world = engine_.getCurrentWorld();

    auto& worldComponent = registry.get<World>(world);
    auto& cameraComponent = registry.get<Camera>(camera);
    auto& positionComponent = registry.get<Position>(camera);

    worldComponent.currentState = worldComponent.defaultState;
    cameraComponent.mode = worldComponent.currentState.camera.mode;

    // TODO: add flag to space for camera transitions
    if (true) {
        auto& cameraScaleAnimator = registry.emplace_or_replace<CameraSizeAnimator>(camera);

        cameraScaleAnimator.timeElapsed = 0.0f;
        cameraScaleAnimator.duration = 1.0f;
        cameraScaleAnimator.targetSize = worldComponent.defaultState.camera.size;
        cameraScaleAnimator.startSize = cameraComponent.size;

        if (worldComponent.defaultState.camera.position.has_value()) {
            auto& cameraPositionAnimator = registry.emplace_or_replace<CameraPositionAnimator>(camera);

            cameraPositionAnimator.timeElapsed = 0.0f;
            cameraPositionAnimator.duration = 1.0f;
            cameraPositionAnimator.startPosition = positionComponent.position;
            cameraPositionAnimator.targetPosition = worldComponent.defaultState.camera.position.value();
        }
    }
}

engine::SpanProxy<::components::TileInstance> engine::EngineAPI::getTileInstances() {
    using namespace ::components;

    return SpanProxy<TileInstance>{engine_.getWorldTilePool().data()};
}

engine::SpanProxy<::components::Proxy<::components::TileInstance>> engine::EngineAPI::getTileGroupProxies(std::uint32_t group) {
    using namespace ::components;

    return SpanProxy<Proxy<TileInstance>>{std::span(engine_.getWorldTilePool().getProxyGroup(group))};
}

void engine::Engine::addScript(const std::string& filepath) {
    luaState_.script_file(filepath);
}

void engine::Engine::runFunction(const std::string& function, std::vector<std::optional<std::string>>& parameters) {
    std::vector<sol::object> luaArgs;

    luaArgs.reserve(parameters.size());

    for (const auto& parameter : parameters) {
        if (parameter.has_value()) {
            luaArgs.push_back(sol::make_object(luaState_, *parameter));
        }
        else {
            luaArgs.push_back(sol::lua_nil);
        }
    }

    luaState_[function](sol::as_args(luaArgs));
}

app::WindowCreateInfo engine::Engine::createWindow() {
    return {
        .context = context_,
        .extent = {1280, 720},
        .title = "Game",
        .visibility = app::WindowVisibility::MINIMISED,
        .resizable = true,
    };
}

void engine::Engine::manageEvents() {
    context_.pollEvents();

    while (window_.hasEvents()) {
        app::WindowEvent event = window_.getNextEvent();

        switch (event.type) {
            case app::WindowEventType::CLOSED:
                running_ = false;
                break;

            case app::WindowEventType::KEY_PRESSED:
                inputManager_.updateKeymaps(event.info.keyPress);
                break;

            case app::WindowEventType::KEY_RELEASED:
                inputManager_.updateKeymaps(event.info.keyRelease);
                break;

            case app::WindowEventType::MOUSE_MOVED:
                inputManager_.updateMousePosition(event.info.mouseMove);
                break;

            case app::WindowEventType::MOUSE_SCROLLED:
                inputManager_.updateMouseScroll(event.info.mouseScroll);
                break;

            default:
                break;
        }
    }
}

void engine::Engine::run() {
    stagingManager_.allocate(renderer_.getImageCounter().count, 4 * 1024 * 1024);

    start();

    std::array<glm::vec2, 4> baseMesh = {
        glm::vec2{1.0, 1.0},
        glm::vec2{0.0, 1.0},
        glm::vec2{1.0, 0.0},
        glm::vec2{0.0, 0.0},
    };

    worldTileMesh_.setBaseMesh(baseMesh);
    entityTileMesh_.setBaseMesh(baseMesh);

    while (running_) {
        inputManager_.update();

        manageEvents();

        auto& stagingBufferFence = stagingManager_.getCurrentFence();

        renderer_.acquireImage({stagingBufferFence});

        if (renderer_.mustAwaitRestore()) {
            if (window_.getVisibility() == app::WindowVisibility::MINIMISED) {
                continue;
            }

            renderer_.disableAwaitRestore();
        }

        update();
        render();

        renderer_.presentImage();
        stagingManager_.rotate();
    }

    close();
}

void engine::Engine::start() {
    std::int32_t tilemapWidth = 0;
    std::int32_t tilemapHeight = 0;
    std::int32_t tilemapChannels = 0;

    std::uint8_t* tilemapImageData = stbi_load("assets/images/tilemap.png", &tilemapWidth, &tilemapHeight, &tilemapChannels, 4);

    for (int i = 0; i < tilemapWidth * tilemapHeight; i++) {
        std::swap(tilemapImageData[i * tilemapChannels + 0], tilemapImageData[i * tilemapChannels + 2]);
    }

    std::int32_t buttonsWidth = 0;
    std::int32_t buttonsHeight = 0;
    std::int32_t buttonsChannels = 0;

    std::uint8_t* buttonsImageData = stbi_load("assets/images/buttons.png", &buttonsWidth, &buttonsHeight, &buttonsChannels, 4);

    for (int i = 0; i < buttonsWidth * buttonsHeight; i++) {
        std::swap(buttonsImageData[i * buttonsChannels + 0], buttonsImageData[i * buttonsChannels + 2]);
    }

    auto& device = renderer_.getDevice();
    auto transferQueue = renderer_.getTransferQueue();

    renderer::SamplerCreateInfo samplerCreateInfo = {
        .device = device,
        .minFilter = renderer::Filter::NEAREST,
        .magFilter = renderer::Filter::NEAREST,
        .mipmapMode = renderer::MipmapMode::NEAREST,
        .addressModeU = renderer::AddressMode::REPEAT,
        .addressModeV = renderer::AddressMode::REPEAT,
        .addressModeW = renderer::AddressMode::REPEAT,
        .borderColour = renderer::BorderColour::FLOAT_OPAQUE_BLACK,
        .maxAnisotropy = {},
        .comparison = {},
        .unnormalisedCoordinates = false,
        .mipLodBias = 0.0f,
        .minLod = 0.0f,
        .maxLod = 0.0f,
    };

    sampler_ = renderer::Sampler::create(samplerCreateInfo);

    renderer::ImageCreateInfo tilemapImageCreateInfo = {
        .device = device,
        .type = renderer::ImageType::IMAGE_2D,
        .format = renderer::ImageFormat::B8G8R8A8_SRGB,
        .memoryType = renderer::MemoryType::DEVICE_LOCAL,
        .usageFlags = renderer::ImageUsageFlags::SAMPLED | renderer::ImageUsageFlags::TRANSFER_DESTINATION,
        .extent = {static_cast<std::uint32_t>(tilemapWidth), static_cast<std::uint32_t>(tilemapHeight), 1},
        .sampleCount = 1,
        .mipLevels = 1,
        .arrayLayers = 1,
    };

    renderer::ImageCreateInfo buttonsImageCreateInfo = {
        .device = device,
        .type = renderer::ImageType::IMAGE_2D,
        .format = renderer::ImageFormat::B8G8R8A8_SRGB,
        .memoryType = renderer::MemoryType::DEVICE_LOCAL,
        .usageFlags = renderer::ImageUsageFlags::SAMPLED | renderer::ImageUsageFlags::TRANSFER_DESTINATION,
        .extent = {static_cast<std::uint32_t>(buttonsWidth), static_cast<std::uint32_t>(buttonsHeight), 1},
        .sampleCount = 1,
        .mipLevels = 1,
        .arrayLayers = 1,
    };

    tilemapImage_ = renderer::Image::create(tilemapImageCreateInfo);
    buttonsImage_ = renderer::Image::create(buttonsImageCreateInfo);

    renderer::CommandPoolCreateInfo commandPoolCreateInfo = {
        .device = device,
        .queue = transferQueue,
    };

    transferCommandPool_ = renderer::CommandPool::create(commandPoolCreateInfo);
    transferCommandBuffers_ = renderer::CommandPool::allocateCommandBuffers(transferCommandPool_, renderer_.getFrameCounter().count);

    renderer::DescriptorSetInputInfo bufferInputInfo = {
        .type = renderer::DescriptorInputType::UNIFORM_BUFFER,
        .stageFlags = renderer::DescriptorShaderStageFlags::VERTEX,
        .count = 1,
        .binding = 0,
    };

    renderer::DescriptorSetInputInfo samplerInputInfo = {
        .type = renderer::DescriptorInputType::IMAGE_SAMPLER,
        .stageFlags = renderer::DescriptorShaderStageFlags::FRAGMENT,
        .count = 1,
        .binding = 1,
    };

    renderer::DescriptorSetLayoutCreateInfo layoutCreateInfo = {
        .device = device,
        .inputs = {bufferInputInfo, samplerInputInfo},
    };

    descriptorSetLayout_ = renderer::DescriptorSetLayout::create(layoutCreateInfo);

    renderer::DescriptorPoolSize bufferSize = {
        .type = renderer::DescriptorInputType::UNIFORM_BUFFER,
        .count = 2,
    };

    renderer::DescriptorPoolSize imageSize = {
        .type = renderer::DescriptorInputType::IMAGE_SAMPLER,
        .count = 2,
    };

    renderer::DescriptorPoolCreateInfo poolCreateInfo = {
        .device = device,
        .poolSizes = {bufferSize, imageSize},
        .maximumSetCount = 2,
    };

    descriptorPool_ = renderer::DescriptorPool::create(poolCreateInfo);

    renderer::BufferCreateInfo cameraBufferCreateInfo = {
        .device = device,
        .memoryType = renderer::MemoryType::DEVICE_LOCAL,
        .usageFlags = renderer::BufferUsageFlags::UNIFORM | renderer::BufferUsageFlags::TRANSFER_DESTINATION,
        .sizeBytes = 2 * sizeof(glm::mat4),
    };

    cameraBuffer_ = renderer::Buffer::create(cameraBufferCreateInfo);

    renderer::DescriptorSetCreateInfo setCreateInfo = {
        .layouts = {descriptorSetLayout_, descriptorSetLayout_},
    };

    descriptorSets_ = renderer::DescriptorPool::allocateDescriptorSets(descriptorPool_, setCreateInfo);

    tilemapDescriptorSet_ = descriptorSets_[0];
    buttonsDescriptorSet_ = descriptorSets_[1];

    auto& transferCommandBuffer = transferCommandBuffers_[renderer_.getFrameCounter().index];
    auto& stagingBuffer = stagingManager_.getCurrentBuffer();

    renderer::Fence temporaryFence = renderer::Fence::create({device, 0});
    renderer::CommandBuffer::beginCapture(transferCommandBuffer);

    renderer::ImageMemoryBarrier tilemapMemoryBarrier0 = {
        .image = tilemapImage_,
        .sourceQueue = {},
        .destinationQueue = {},
        .oldLayout = renderer::ImageLayout::UNDEFINED,
        .newLayout = renderer::ImageLayout::TRANSFER_DESTINATION_OPTIMAL,
        .baseMipLevel = 0,
        .mipLevelCount = 1,
        .baseArrayLayer = 0,
        .arrayLayerCount = 1,
        .aspectMask = renderer::ImageAspectFlags::COLOUR,
        .sourceAccessFlags = renderer::AccessFlags::NONE,
        .destinationAccessFlags = renderer::AccessFlags::TRANSFER_WRITE,
    };

    renderer::ImageMemoryBarrier buttonsMemoryBarrier0 = {
        .image = buttonsImage_,
        .sourceQueue = {},
        .destinationQueue = {},
        .oldLayout = renderer::ImageLayout::UNDEFINED,
        .newLayout = renderer::ImageLayout::TRANSFER_DESTINATION_OPTIMAL,
        .baseMipLevel = 0,
        .mipLevelCount = 1,
        .baseArrayLayer = 0,
        .arrayLayerCount = 1,
        .aspectMask = renderer::ImageAspectFlags::COLOUR,
        .sourceAccessFlags = renderer::AccessFlags::NONE,
        .destinationAccessFlags = renderer::AccessFlags::TRANSFER_WRITE,
    };

    renderer::CommandBuffer::pipelineBarrier(transferCommandBuffer, renderer::PipelineStageFlags::TOP_OF_PIPE, renderer::PipelineStageFlags::TRANSFER, {tilemapMemoryBarrier0, buttonsMemoryBarrier0});

    std::size_t totalSize = renderer::Image::getSize(tilemapImage_) + renderer::Image::getSize(buttonsImage_);

    auto mapping = renderer::Buffer::map(stagingBuffer, totalSize, stagingManager_.getOffset());

    std::memcpy(mapping.data.data(), tilemapImageData, renderer::Image::getSize(tilemapImage_));
    std::memcpy(mapping.data.data() + renderer::Image::getSize(tilemapImage_), buttonsImageData, renderer::Image::getSize(buttonsImage_));

    renderer::Buffer::unmap(stagingBuffer, mapping);

    renderer::BufferImageCopyRegion tilemapCopyRegion = {
        .bufferOffset = stagingManager_.getOffset(),
        .bufferRowLength = 0,
        .bufferImageHeight = 0,
        .mipLevel = 0,
        .baseArrayLayer = 0,
        .arrayLayerCount = 1,
        .imageAspectMask = renderer::ImageAspectFlags::COLOUR,
        .imageOffset = {0, 0, 0},
        .imageExtent = {static_cast<std::uint32_t>(tilemapWidth), static_cast<std::uint32_t>(tilemapHeight), 1},
    };

    stagingManager_.getOffset() += renderer::Image::getSize(tilemapImage_);

    renderer::BufferImageCopyRegion buttonsCopyRegion = {
        .bufferOffset = stagingManager_.getOffset(),
        .bufferRowLength = 0,
        .bufferImageHeight = 0,
        .mipLevel = 0,
        .baseArrayLayer = 0,
        .arrayLayerCount = 1,
        .imageAspectMask = renderer::ImageAspectFlags::COLOUR,
        .imageOffset = {0, 0, 0},
        .imageExtent = {static_cast<std::uint32_t>(buttonsWidth), static_cast<std::uint32_t>(buttonsHeight), 1},
    };

    stagingManager_.getOffset() += renderer::Image::getSize(buttonsImage_);

    renderer::CommandBuffer::copyBufferToImage(transferCommandBuffer, stagingBuffer, tilemapImage_, renderer::ImageLayout::TRANSFER_DESTINATION_OPTIMAL, {tilemapCopyRegion});
    renderer::CommandBuffer::copyBufferToImage(transferCommandBuffer, stagingBuffer, buttonsImage_, renderer::ImageLayout::TRANSFER_DESTINATION_OPTIMAL, {buttonsCopyRegion});

    renderer::ImageMemoryBarrier tilemapMemoryBarrier1 = {
        .image = tilemapImage_,
        .sourceQueue = {},
        .destinationQueue = {},
        .oldLayout = renderer::ImageLayout::TRANSFER_DESTINATION_OPTIMAL,
        .newLayout = renderer::ImageLayout::SHADER_READ_ONLY_OPTIMAL,
        .baseMipLevel = 0,
        .mipLevelCount = 1,
        .baseArrayLayer = 0,
        .arrayLayerCount = 1,
        .aspectMask = renderer::ImageAspectFlags::COLOUR,
        .sourceAccessFlags = renderer::AccessFlags::TRANSFER_WRITE,
        .destinationAccessFlags = renderer::AccessFlags::SHADER_READ,
    };

    renderer::ImageMemoryBarrier buttonsMemoryBarrier1 = {
        .image = buttonsImage_,
        .sourceQueue = {},
        .destinationQueue = {},
        .oldLayout = renderer::ImageLayout::TRANSFER_DESTINATION_OPTIMAL,
        .newLayout = renderer::ImageLayout::SHADER_READ_ONLY_OPTIMAL,
        .baseMipLevel = 0,
        .mipLevelCount = 1,
        .baseArrayLayer = 0,
        .arrayLayerCount = 1,
        .aspectMask = renderer::ImageAspectFlags::COLOUR,
        .sourceAccessFlags = renderer::AccessFlags::TRANSFER_WRITE,
        .destinationAccessFlags = renderer::AccessFlags::SHADER_READ,
    };

    renderer::CommandBuffer::pipelineBarrier(transferCommandBuffer, renderer::PipelineStageFlags::TRANSFER, renderer::PipelineStageFlags::FRAGMENT_SHADER, {tilemapMemoryBarrier1, buttonsMemoryBarrier1});

    using namespace ::components;

    currentWorld_ = registry_.create();

    auto& worldComponent = registry_.emplace<World>(currentWorld_);

    worldComponent.defaultState.path = "assets/worlds/default";

    currentEntity_ = registry_.create();

    auto& controller = registry_.emplace<PositionController>(currentEntity_);

    controller.forwardBinding = app::Key::W;
    controller.backwardBinding = app::Key::S;
    controller.leftBinding = app::Key::A;
    controller.rightBinding = app::Key::D;

    auto proxy = entityTilePool_.insert({
        .transform = {
            .position = {0.0, 0.0, -0.5},
            .scale = {1.0, 1.0},
        },
        .appearance = {
            .texture = {
                .sample = {
                    .position = {0.0, 0.0},
                    .extent = {0.2, 0.2},
                },
                .offset = {0.0, 0.0},
                .repeat = {1.0, 1.0},
            },
            .colourFactor = {1.0, 1.0, 1.0},

        },
    });

    registry_.emplace<TileProxy>(currentEntity_, proxy);
    registry_.emplace<Position>(currentEntity_, glm::vec3{0.0, 0.0, 0.0});
    registry_.emplace<Scale>(currentEntity_, glm::vec2{1.0, 1.0});
    registry_.emplace<TileTag>(currentEntity_);
    registry_.emplace<EntityTag>(currentEntity_);
    registry_.emplace<CurrentEntityTag>(currentEntity_);

    ::systems::loadWorlds(registry_, *this);
    ::systems::entities::createEntities(*this);

    entityTileMesh_.createInstanceBuffer(1);
    worldTileMesh_.createInstanceBuffer(worldComponent.tiles.size());

    currentCamera_ = registry_.create();

    auto& cameraComponent = registry_.emplace<Camera>(currentCamera_);
    auto& cameraPosition = registry_.emplace<Position>(currentCamera_);
    auto& cameraScale = registry_.emplace<Scale>(currentCamera_);

    cameraComponent.near = -1.0f;
    cameraComponent.far = 1.0f;
    cameraScale.scale = window_.extent();

    cameraComponent.mode = worldComponent.defaultState.camera.mode;
    cameraComponent.size = worldComponent.defaultState.camera.size;

    if (worldComponent.defaultState.camera.position.has_value()) {
        cameraPosition.position = worldComponent.defaultState.camera.position.value();
    }
    else {
        cameraPosition.position = {0.0f, 0.0f};
    }

    renderer::CommandBuffer::endCapture(transferCommandBuffer);

    renderer::QueueSubmitInfo submitInfo = {
        .fence = temporaryFence,
        .commandBuffers = {transferCommandBuffer},
        .waits = {},
        .signals = {},
        .waitFlags = {},
    };

    renderer::Queue::submit(transferQueue, submitInfo);
    renderer::Device::waitForFences(device, {temporaryFence});
    renderer::Fence::destroy(temporaryFence);

    createBasicPipelineResources();

    renderer::DescriptorSetBufferBinding cameraBufferBinding = {
        .buffer = cameraBuffer_,
        .offsetBytes = 0,
        .rangeBytes = renderer::Buffer::size(cameraBuffer_),
    };

    renderer::DescriptorSetUpdateInfo tilemapUniformBufferUpdateInfo = {
        .set = tilemapDescriptorSet_,
        .inputType = renderer::DescriptorInputType::UNIFORM_BUFFER,
        .binding = 0,
        .arrayElement = 0,
        .buffers = {cameraBufferBinding},
        .images = {},
    };

    renderer::DescriptorSetUpdateInfo buttonsUniformBufferUpdateInfo = {
        .set = buttonsDescriptorSet_,
        .inputType = renderer::DescriptorInputType::UNIFORM_BUFFER,
        .binding = 0,
        .arrayElement = 0,
        .buffers = {cameraBufferBinding},
        .images = {},
    };

    renderer::DescriptorPool::updateDescriptorSets(descriptorPool_, {tilemapUniformBufferUpdateInfo, buttonsUniformBufferUpdateInfo});

    renderer::ImageViewCreateInfo tilemapImageViewCreateInfo = {
        .image = tilemapImage_,
        .type = renderer::ImageViewType::IMAGE_2D,
        .aspectFlags = renderer::ImageAspectFlags::COLOUR,
        .baseMipLevel = 0,
        .levelCount = 1,
        .baseArrayLayer = 0,
        .layerCount = 1,
    };

    renderer::ImageViewCreateInfo buttonsImageViewCreateInfo = {
        .image = buttonsImage_,
        .type = renderer::ImageViewType::IMAGE_2D,
        .aspectFlags = renderer::ImageAspectFlags::COLOUR,
        .baseMipLevel = 0,
        .levelCount = 1,
        .baseArrayLayer = 0,
        .layerCount = 1,
    };

    tilemapImageView_ = renderer::ImageView::create(tilemapImageViewCreateInfo);
    buttonsImageView_ = renderer::ImageView::create(buttonsImageViewCreateInfo);

    lastFrameTime_ = std::chrono::high_resolution_clock::now();

    stbi_image_free(tilemapImageData);
    stbi_image_free(buttonsImageData);

    renderer::DescriptorSetImageBinding worldTilesSamplerBinding = {
        .image = tilemapImageView_,
        .sampler = sampler_,
        .layout = renderer::ImageLayout::SHADER_READ_ONLY_OPTIMAL,
    };

    renderer::DescriptorSetUpdateInfo worldTilesSamplerUpdateInfo = {
        .set = tilemapDescriptorSet_,
        .inputType = renderer::DescriptorInputType::IMAGE_SAMPLER,
        .binding = 1,
        .arrayElement = 0,
        .buffers = {},
        .images = {worldTilesSamplerBinding},
    };

    renderer::DescriptorPool::updateDescriptorSets(descriptorPool_, {worldTilesSamplerUpdateInfo});

    renderer::DescriptorSetImageBinding buttonTilesSamplerBinding = {
        .image = buttonsImageView_,
        .sampler = sampler_,
        .layout = renderer::ImageLayout::SHADER_READ_ONLY_OPTIMAL,
    };

    renderer::DescriptorSetUpdateInfo buttonTilesSamplerUpdateInfo = {
        .set = buttonsDescriptorSet_,
        .inputType = renderer::DescriptorInputType::IMAGE_SAMPLER,
        .binding = 1,
        .arrayElement = 0,
        .buffers = {},
        .images = {buttonTilesSamplerBinding},
    };

    renderer::DescriptorPool::updateDescriptorSets(descriptorPool_, {buttonTilesSamplerUpdateInfo});
}

void engine::Engine::calculateDeltaTime() {
    thisFrameTime_ = std::chrono::high_resolution_clock::now();
    deltaTime_ = std::clamp(std::chrono::duration<float>(thisFrameTime_ - lastFrameTime_).count(), 0.0f, 0.1f);
    lastFrameTime_ = thisFrameTime_;
}

void engine::Engine::runPreTransferSystems() {
    using namespace ::components;

    auto& cameraScale = registry_.get<Scale>(currentCamera_);
    cameraScale.scale = window_.extent();

    ::systems::entities::updateControllers(*this);

    ::systems::integrateMovements(registry_, deltaTime_);

    ::systems::testCollisions(registry_);
    ::systems::checkTriggers(registry_);
    ::systems::performTriggers(registry_, *this, deltaTime_);

    ::systems::cameras::animateCameraPositions(*this);
    ::systems::cameras::animateCameraSizes(*this);
    ::systems::cameras::makeCamerasFollowTarget(*this);
    ::systems::cameras::calculateCameraData(*this);

    ::systems::transformInstances(registry_, entityTilePool_.data());
}

void engine::Engine::runMidTransferSystems() {
    entityTileMesh_.setInstances(entityTilePool_.data());
    worldTileMesh_.setInstances(worldTilePool_.data());

    ::systems::cameras::uploadCameraData(*this);
}

void engine::Engine::runPostTransferSystems() {
    using namespace ::components;
    ::systems::cacheLasts<Position>(registry_);
    ::systems::cacheLasts<Velocity>(registry_);
    ::systems::cacheLasts<Acceleration>(registry_);
}

void engine::Engine::update() {
    auto frameIndex = renderer_.getFrameCounter().index;
    auto& transferQueue = renderer_.getTransferQueue();

    auto& transferCommandBuffer = transferCommandBuffers_[frameIndex];
    auto& stagingBufferFence = stagingManager_.getCurrentFence();
    auto& stagingBufferSemaphore = stagingManager_.getCurrentSemaphore();

    renderer::CommandBuffer::reset(transferCommandBuffer);

    calculateDeltaTime();
    runPreTransferSystems();

    renderer::CommandBuffer::beginCapture(transferCommandBuffer);

    runMidTransferSystems();

    renderer::CommandBuffer::endCapture(transferCommandBuffer);

    runPostTransferSystems();

    renderer::QueueSubmitInfo submitInfo = {
        .fence = stagingBufferFence,
        .commandBuffers = {transferCommandBuffer},
        .waits = {},
        .signals = {stagingBufferSemaphore},
        .waitFlags = {},
    };

    renderer::Queue::submit(transferQueue, submitInfo);
}

void engine::Engine::render() {
    auto& commandBuffer = renderer_.getCurrentCommandBuffer();
    auto& inFlightFence = renderer_.getCurrentInFlightFence();
    auto& acquireSemaphore = renderer_.getCurrentAcquireSemaphore();
    auto& framebuffer = renderer_.getCurrentFramebuffer();
    auto& presentSemaphore = renderer_.getCurrentPresentSemaphore();
    auto& swapchain = renderer_.getSwapchain();
    auto& renderPass = renderer_.getRenderPass();
    auto& graphicsQueue = renderer_.getGraphicsQueue();

    auto& stagingBufferSemaphore = stagingManager_.getCurrentSemaphore();

    renderer::CommandBuffer::reset(commandBuffer);

    renderer::Viewport viewport = {
        .position = {0.0, 0.0},
        .extent = renderer::Swapchain::getExtent(swapchain),
        .minDepth = 0.0f,
        .maxDepth = 1.0f,
    };

    renderer::Scissor scissor = {
        .offset = {0, 0},
        .extent = renderer::Swapchain::getExtent(swapchain),
    };

    renderer::RenderPassBeginInfo renderPassBeginInfo = {
        .renderPass = renderPass,
        .framebuffer = framebuffer,
        .region = {
            .position = {0, 0},
            .extent = renderer::Swapchain::getExtent(swapchain),
        },
        .colourClearValues = {
            glm::fvec4{0.0, 0.0, 0.0, 1.0},
        },
        .depthClearValue = 1.0f,
        .stencilClearValue = 0xFF,
    };

    using namespace ::components;

    renderer::CommandBuffer::beginCapture(commandBuffer);
    renderer::CommandBuffer::beginRenderPass(commandBuffer, renderPassBeginInfo);

    renderer::CommandBuffer::bindPipeline(commandBuffer, worldPipeline_);
    renderer::CommandBuffer::setPipelineViewports(commandBuffer, {viewport}, 0);
    renderer::CommandBuffer::setPipelineScissors(commandBuffer, {scissor}, 0);

    renderer::CommandBuffer::bindVertexBuffers(commandBuffer, {worldTileMesh_.getMeshBuffer(), worldTileMesh_.getInstanceBuffer()}, {0, 0}, 0);

    renderer::CommandBuffer::bindDescriptorSets(commandBuffer, renderer::DeviceOperation::GRAPHICS, basicPipelineLayout_, 0, {tilemapDescriptorSet_});
    renderer::CommandBuffer::draw(commandBuffer, 4, static_cast<std::uint32_t>(worldTilePool_.data().size()), 0, 0);
    renderer::CommandBuffer::bindVertexBuffers(commandBuffer, {entityTileMesh_.getMeshBuffer(), entityTileMesh_.getInstanceBuffer()}, {0, 0}, 0);
    renderer::CommandBuffer::draw(commandBuffer, 4, static_cast<std::uint32_t>(entityTilePool_.data().size()), 0, 0);

    renderer::CommandBuffer::bindPipeline(commandBuffer, uiPipeline_);
    renderer::CommandBuffer::endRenderPass(commandBuffer);
    renderer::CommandBuffer::endCapture(commandBuffer);

    renderer::QueueSubmitInfo submitInfo = {
        .fence = inFlightFence,
        .commandBuffers = {commandBuffer},
        .waits = {stagingBufferSemaphore, acquireSemaphore},
        .signals = {presentSemaphore},
        .waitFlags = {
            renderer::PipelineStageFlags::VERTEX_INPUT,
            renderer::PipelineStageFlags::COLOR_ATTACHMENT_OUTPUT,
        },
    };

    renderer::Queue::submit(graphicsQueue, submitInfo);
}

void engine::Engine::close() {
    auto& device = renderer_.getDevice();

    renderer::Device::waitIdle(device);

    renderer::CommandPool::destroy(transferCommandPool_);
    renderer::DescriptorPool::destroy(descriptorPool_);
    renderer::DescriptorSetLayout::destroy(descriptorSetLayout_);
    renderer::Pipeline::destroy(worldPipeline_);
    renderer::Pipeline::destroy(uiPipeline_);
    renderer::PipelineLayout::destroy(basicPipelineLayout_);

    renderer::ImageView::destroy(tilemapImageView_);
    renderer::ImageView::destroy(buttonsImageView_);

    renderer::Image::destroy(tilemapImage_);
    renderer::Image::destroy(buttonsImage_);

    renderer::Sampler::destroy(sampler_);
}

void engine::Engine::createBasicPipelineResources() {
    auto& device = renderer_.getDevice();
    auto renderPass = renderer_.getRenderPass();

    std::ifstream uiVertShader("assets/shaders/ui.vert.spv", std::ios::binary | std::ios::ate);
    std::ifstream tileVertShader("assets/shaders/tile.vert.spv", std::ios::binary | std::ios::ate);
    std::ifstream tileFragShader("assets/shaders/tile.frag.spv", std::ios::binary | std::ios::ate);

    std::uint64_t uiVertShaderSize = static_cast<std::uint64_t>(uiVertShader.tellg());
    std::uint64_t tileVertShaderSize = static_cast<std::uint64_t>(tileVertShader.tellg());
    std::uint64_t tileFragShaderSize = static_cast<std::uint64_t>(tileFragShader.tellg());

    uiVertShader.seekg(0, std::ios::beg);
    tileVertShader.seekg(0, std::ios::beg);
    tileFragShader.seekg(0, std::ios::beg);

    std::vector<std::uint32_t> uiVertShaderBinary(uiVertShaderSize / sizeof(std::uint32_t));
    std::vector<std::uint32_t> tileVertShaderBinary(tileVertShaderSize / sizeof(std::uint32_t));
    std::vector<std::uint32_t> tileFragShaderBinary(tileFragShaderSize / sizeof(std::uint32_t));

    uiVertShader.read(reinterpret_cast<char*>(uiVertShaderBinary.data()), static_cast<std::uint32_t>(uiVertShaderSize));
    tileVertShader.read(reinterpret_cast<char*>(tileVertShaderBinary.data()), static_cast<std::uint32_t>(tileVertShaderSize));
    tileFragShader.read(reinterpret_cast<char*>(tileFragShaderBinary.data()), static_cast<std::uint32_t>(tileFragShaderSize));

    renderer::ShaderModuleCreateInfo uiVertShaderModuleCreateInfo = {
        .device = device,
        .data = uiVertShaderBinary,
    };

    renderer::ShaderModuleCreateInfo tileVertShaderModuleCreateInfo = {
        .device = device,
        .data = tileVertShaderBinary,
    };

    renderer::ShaderModuleCreateInfo tileFragShaderModuleCreateInfo = {
        .device = device,
        .data = tileFragShaderBinary,
    };

    renderer::ShaderModule uiVertShaderModule = renderer::ShaderModule::create(uiVertShaderModuleCreateInfo);
    renderer::ShaderModule tileVertShaderModule = renderer::ShaderModule::create(tileVertShaderModuleCreateInfo);
    renderer::ShaderModule tileFragShaderModule = renderer::ShaderModule::create(tileFragShaderModuleCreateInfo);

    renderer::PipelineLayoutCreateInfo pipelineLayoutCreateInfo = {
        .device = device,
        .inputLayouts = {descriptorSetLayout_},
        .pushConstants = {},
    };

    basicPipelineLayout_ = renderer::PipelineLayout::create(pipelineLayoutCreateInfo);

    renderer::PipelineCreateInfo worldPipelineCreateInfo = {
        .renderPass = renderPass,
        .layout = basicPipelineLayout_,
        .subpassIndex = 0,
        .shaderStages = {
            {
                tileVertShaderModule,
                renderer::ShaderStage::VERTEX,
                "main",
            },
            {
                tileFragShaderModule,
                renderer::ShaderStage::FRAGMENT,
                "main",
            },
        },
        .vertexInput = {
            .bindings = {
                renderer::VertexInputBindingDescription{
                    .inputRate = renderer::VertexInputRate::PER_VERTEX,
                    .binding = 0,
                    .strideBytes = sizeof(glm::vec2),
                },
                renderer::VertexInputBindingDescription{
                    .inputRate = renderer::VertexInputRate::PER_INSTANCE,
                    .binding = 1,
                    .strideBytes = sizeof(::components::TileInstance),
                },
            },
            .attributes = {
                // === VERTEX ===
                renderer::VertexAttributeDescription{
                    .format = renderer::VertexAttributeFormat::R32G32_FLOAT,
                    .binding = 0,
                    .location = 0,
                },
                // === POSITION ===
                renderer::VertexAttributeDescription{
                    .format = renderer::VertexAttributeFormat::R32G32B32_FLOAT,
                    .binding = 1,
                    .location = 1,
                },
                // === SCALE ===
                renderer::VertexAttributeDescription{
                    .format = renderer::VertexAttributeFormat::R32G32_FLOAT,
                    .binding = 1,
                    .location = 2,
                },
                // === TEXTURE POSITION ===
                renderer::VertexAttributeDescription{
                    .format = renderer::VertexAttributeFormat::R32G32_FLOAT,
                    .binding = 1,
                    .location = 3,
                },
                // === TEXTURE EXTENT ===
                renderer::VertexAttributeDescription{
                    .format = renderer::VertexAttributeFormat::R32G32_FLOAT,
                    .binding = 1,
                    .location = 4,
                },
                // === TEXTURE OFFSET ===
                renderer::VertexAttributeDescription{
                    .format = renderer::VertexAttributeFormat::R32G32_FLOAT,
                    .binding = 1,
                    .location = 5,
                },
                // === TEXTURE REPEAT ===
                renderer::VertexAttributeDescription{
                    .format = renderer::VertexAttributeFormat::R32G32_FLOAT,
                    .binding = 1,
                    .location = 6,
                },
                // === COLOUR FACTOR ===
                renderer::VertexAttributeDescription{
                    .format = renderer::VertexAttributeFormat::R32G32B32_FLOAT,
                    .binding = 1,
                    .location = 7,
                },
            },
        },
        .inputAssembly = {
            .topology = renderer::PolygonTopology::TRIANGLE_STRIP,
            .primitiveRestart = false,
        },
        .viewportCount = 1,
        .scissorCount = 1,
        .rasterisation = {
            .frontFaceWinding = renderer::PolygonFaceWinding::ANTICLOCKWISE,
            .cullMode = renderer::PolygonCullMode::NEVER,
            .frontface = {
                .depthComparison = renderer::CompareOperation::LESS_EQUAL,
                .stencilComparison = renderer::CompareOperation::ALWAYS,
                .stencilFailOperation = renderer::ValueOperation::KEEP,
                .depthFailOperation = renderer::ValueOperation::KEEP,
                .passOperation = renderer::ValueOperation::KEEP,
                .stencilCompareMask = 0xFF,
                .stencilWriteMask = 0xFF,
            },
            .backface = {
                .depthComparison = renderer::CompareOperation::LESS_EQUAL,
                .stencilComparison = renderer::CompareOperation::ALWAYS,
                .stencilFailOperation = renderer::ValueOperation::KEEP,
                .depthFailOperation = renderer::ValueOperation::KEEP,
                .passOperation = renderer::ValueOperation::KEEP,
                .stencilCompareMask = 0xFF,
                .stencilWriteMask = 0xFF,
            },
            .depthClampEnable = false,
            .depthTestEnable = true,
            .depthWriteEnable = true,
            .depthBoundsTestEnable = false,
            .stencilTestEnable = false,
        },
        .multisample = {},
        .colourBlend = {
            .attachments = {renderer::ColourBlendAttachment{}},
        },
    };

    renderer::PipelineCreateInfo uiPipelineCreateInfo = {
        .renderPass = renderPass,
        .layout = basicPipelineLayout_,
        .subpassIndex = 0,
        .shaderStages = {
            {
                uiVertShaderModule,
                renderer::ShaderStage::VERTEX,
                "main",
            },
            {
                tileFragShaderModule,
                renderer::ShaderStage::FRAGMENT,
                "main",
            },
        },
        .vertexInput = {
            .bindings = {
                renderer::VertexInputBindingDescription{
                    .inputRate = renderer::VertexInputRate::PER_VERTEX,
                    .binding = 0,
                    .strideBytes = sizeof(glm::vec2),
                },
                renderer::VertexInputBindingDescription{
                    .inputRate = renderer::VertexInputRate::PER_INSTANCE,
                    .binding = 1,
                    .strideBytes = sizeof(::components::TileInstance),
                },
            },
            .attributes = {
                // === VERTEX ===
                renderer::VertexAttributeDescription{
                    .format = renderer::VertexAttributeFormat::R32G32_FLOAT,
                    .binding = 0,
                    .location = 0,
                },
                // === POSITION ===
                renderer::VertexAttributeDescription{
                    .format = renderer::VertexAttributeFormat::R32G32B32_FLOAT,
                    .binding = 1,
                    .location = 1,
                },
                // === SCALE ===
                renderer::VertexAttributeDescription{
                    .format = renderer::VertexAttributeFormat::R32G32_FLOAT,
                    .binding = 1,
                    .location = 2,
                },
                // === TEXTURE POSITION ===
                renderer::VertexAttributeDescription{
                    .format = renderer::VertexAttributeFormat::R32G32_FLOAT,
                    .binding = 1,
                    .location = 3,
                },
                // === TEXTURE EXTENT ===
                renderer::VertexAttributeDescription{
                    .format = renderer::VertexAttributeFormat::R32G32_FLOAT,
                    .binding = 1,
                    .location = 4,
                },
                // === TEXTURE OFFSET ===
                renderer::VertexAttributeDescription{
                    .format = renderer::VertexAttributeFormat::R32G32_FLOAT,
                    .binding = 1,
                    .location = 5,
                },
                // === TEXTURE REPEAT ===
                renderer::VertexAttributeDescription{
                    .format = renderer::VertexAttributeFormat::R32G32_FLOAT,
                    .binding = 1,
                    .location = 6,
                },
                // === COLOUR FACTOR ===
                renderer::VertexAttributeDescription{
                    .format = renderer::VertexAttributeFormat::R32G32B32_FLOAT,
                    .binding = 1,
                    .location = 7,
                },
            },
        },
        .inputAssembly = {
            .topology = renderer::PolygonTopology::TRIANGLE_STRIP,
            .primitiveRestart = false,
        },
        .viewportCount = 1,
        .scissorCount = 1,
        .rasterisation = {
            .frontFaceWinding = renderer::PolygonFaceWinding::ANTICLOCKWISE,
            .cullMode = renderer::PolygonCullMode::NEVER,
            .frontface = {
                .depthComparison = renderer::CompareOperation::LESS_EQUAL,
                .stencilComparison = renderer::CompareOperation::ALWAYS,
                .stencilFailOperation = renderer::ValueOperation::KEEP,
                .depthFailOperation = renderer::ValueOperation::KEEP,
                .passOperation = renderer::ValueOperation::KEEP,
                .stencilCompareMask = 0xFF,
                .stencilWriteMask = 0xFF,
            },
            .backface = {
                .depthComparison = renderer::CompareOperation::LESS_EQUAL,
                .stencilComparison = renderer::CompareOperation::ALWAYS,
                .stencilFailOperation = renderer::ValueOperation::KEEP,
                .depthFailOperation = renderer::ValueOperation::KEEP,
                .passOperation = renderer::ValueOperation::KEEP,
                .stencilCompareMask = 0xFF,
                .stencilWriteMask = 0xFF,
            },
            .depthClampEnable = false,
            .depthTestEnable = true,
            .depthWriteEnable = true,
            .depthBoundsTestEnable = false,
            .stencilTestEnable = false,
        },
        .multisample = {},
        .colourBlend = {
            .attachments = {renderer::ColourBlendAttachment{}},
        },
    };

    pipelines_ = renderer::Device::createPipelines(device, {worldPipelineCreateInfo, uiPipelineCreateInfo});

    worldPipeline_ = pipelines_[0];
    uiPipeline_ = pipelines_[1];

    renderer::ShaderModule::destroy(uiVertShaderModule);
    renderer::ShaderModule::destroy(tileVertShaderModule);
    renderer::ShaderModule::destroy(tileFragShaderModule);
}