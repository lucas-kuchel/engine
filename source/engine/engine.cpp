#include <engine/engine.hpp>

#include <engine/components/entity_tags.hpp>
#include <engine/components/space.hpp>

#include <engine/components/proxy.hpp>

#include <engine/components/buttons.hpp>
#include <engine/systems/buttons.hpp>

#include <engine/components/character.hpp>
#include <engine/systems/character.hpp>

#include <engine/components/transforms.hpp>
#include <engine/systems/transforms.hpp>

#include <engine/components/controllers.hpp>
#include <engine/systems/controllers.hpp>

#include <engine/components/tile.hpp>
#include <engine/systems/tile.hpp>

#include <engine/components/camera.hpp>
#include <engine/systems/camera.hpp>

#include <engine/components/world.hpp>
#include <engine/systems/world.hpp>

#include <fstream>

#include <glm/gtc/matrix_transform.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

engine::Engine::Engine()
    : api_(*this), window_(createWindow()), renderer_(window_) {
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

    luaState_.new_usertype<TileProxy>(
        "TileProxy",
        "index", &TileProxy::index);

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

    luaState_.new_usertype<components::Camera>(
        "CameraState",
        "near", &components::Camera::near,
        "far", &components::Camera::far,
        "scale", &components::Camera::scale,
        "mode", &components::Camera::mode);

    luaState_.new_enum("CameraMode",
                       "FOLLOW", components::CameraMode::FOLLOW,
                       "LOCKED", components::CameraMode::LOCKED);

    luaState_.new_usertype<SpanProxy<TileInstance>>(
        "TileInstanceSpan",
        sol::meta_function::length, &SpanProxy<TileInstance>::size,
        sol::meta_function::index, &SpanProxy<TileInstance>::get,
        "__len", &SpanProxy<TileInstance>::size,
        "get", &SpanProxy<TileInstance>::get);

    luaState_.new_usertype<SpanProxy<TileProxy>>(
        "TileProxySpan",
        sol::meta_function::length, &SpanProxy<TileProxy>::size,
        sol::meta_function::index, &SpanProxy<TileProxy>::get,
        "__len", &SpanProxy<TileProxy>::size,
        "get", &SpanProxy<TileProxy>::get);

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

void engine::EngineAPI::addToGroup(const TileProxy& proxy, std::uint32_t group) {
    auto& list = engine_.sparseTileGroups_[static_cast<std::size_t>(group)];

    for (const auto& entry : list) {
        if (entry.index == proxy.index) {
            return;
        }
    }

    list.push_back(proxy);
}

engine::CameraInfo engine::EngineAPI::getCameraInfo() {
    return {
        .state = engine_.registry_.get<components::Camera>(engine_.currentCamera_),
        .rotation = engine_.registry_.get<components::Rotation>(engine_.currentCamera_).angle,
        .scale = engine_.registry_.get<components::Scale>(engine_.currentCamera_).scale,
        .position = engine_.registry_.get<components::Position>(engine_.currentCamera_).position,
    };
}

void engine::EngineAPI::setCameraInfo(CameraInfo& cameraInfo) {
    auto& state = engine_.registry_.get<components::Camera>(engine_.currentCamera_);
    auto& rotation = engine_.registry_.get<components::Rotation>(engine_.currentCamera_).angle;
    auto& scale = engine_.registry_.get<components::Scale>(engine_.currentCamera_).scale;
    auto& position = engine_.registry_.get<components::Position>(engine_.currentCamera_).position;

    state = cameraInfo.state;
    rotation = cameraInfo.rotation;
    scale = cameraInfo.scale;
    position = cameraInfo.position;
}

void engine::EngineAPI::removeFromGroup(const TileProxy& proxy, std::uint32_t group) {
    auto& list = engine_.sparseTileGroups_[static_cast<std::size_t>(group)];

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
    return engine_.deltaTime_;
}

void engine::EngineAPI::setSpace(const std::string& space) {
    auto& world = engine_.registry_.get<components::World>(engine_.currentWorld_);
    auto& camera = engine_.registry_.get<components::Camera>(engine_.currentCamera_);
    auto& position = engine_.registry_.get<components::Position>(engine_.currentCamera_);

    for (auto& spaceEntity : world.spaces) {
        auto& spaceComponent = engine_.registry_.get<components::Space>(spaceEntity);

        if (spaceComponent.name != space) {
            continue;
        }

        world.currentState.camera.mode = spaceComponent.camera.mode;
        world.currentState.camera.scale = spaceComponent.camera.scale;

        world.currentState.physics.kineticFriction = spaceComponent.physics.kineticFriction;
        world.currentState.physics.staticFriction = spaceComponent.physics.staticFriction;

        // TODO: move to camera update
        camera.mode = spaceComponent.camera.mode;

        // TODO: add flag to space for camera transitions
        if (true) {
            auto& cameraScaleAnimator = engine_.registry_.emplace_or_replace<components::CameraScaleAnimator>(engine_.currentCamera_);
            cameraScaleAnimator.timeElapsed = 0.0f;
            cameraScaleAnimator.duration = 1.0f;
            cameraScaleAnimator.targetScale = spaceComponent.camera.scale;
            cameraScaleAnimator.startScale = camera.scale;

            if (spaceComponent.camera.position.has_value()) {
                auto& cameraPositionAnimator = engine_.registry_.emplace_or_replace<components::CameraPositionAnimator>(engine_.currentCamera_);

                cameraPositionAnimator.timeElapsed = 0.0f;
                cameraPositionAnimator.duration = 1.0f;
                cameraPositionAnimator.startPosition = position.position;
                cameraPositionAnimator.targetPosition = spaceComponent.camera.position.value();
                world.currentState.camera.position = spaceComponent.camera.position.value();
            }
        }
        else {
            camera.scale = spaceComponent.camera.scale;

            if (spaceComponent.camera.position.has_value()) {
                position.position = spaceComponent.camera.position.value();
            }
        }

        break;
    }
}

void engine::EngineAPI::resetSpace() {
    auto& world = engine_.registry_.get<components::World>(engine_.currentWorld_);
    auto& camera = engine_.registry_.get<components::Camera>(engine_.currentCamera_);
    auto& position = engine_.registry_.get<components::Position>(engine_.currentCamera_);

    world.currentState = world.defaultState;

    camera.mode = world.currentState.camera.mode;

    // TODO: add flag to space for camera transitions
    if (true) {
        auto& cameraScaleAnimator = engine_.registry_.emplace_or_replace<components::CameraScaleAnimator>(engine_.currentCamera_);

        cameraScaleAnimator.timeElapsed = 0.0f;
        cameraScaleAnimator.duration = 1.0f;
        cameraScaleAnimator.targetScale = world.defaultState.camera.scale;
        cameraScaleAnimator.startScale = camera.scale;

        if (world.defaultState.camera.position.has_value()) {
            auto& cameraPositionAnimator = engine_.registry_.emplace_or_replace<components::CameraPositionAnimator>(engine_.currentCamera_);

            cameraPositionAnimator.timeElapsed = 0.0f;
            cameraPositionAnimator.duration = 1.0f;
            cameraPositionAnimator.startPosition = position.position;
            cameraPositionAnimator.targetPosition = world.defaultState.camera.position.value();
        }
    }
}

engine::SpanProxy<engine::TileInstance> engine::EngineAPI::getTileInstances() {
    return SpanProxy<TileInstance>{std::span(engine_.tiles_)};
}

engine::SpanProxy<engine::TileProxy> engine::EngineAPI::getTileGroupProxies(std::uint32_t group) {
    return SpanProxy<TileProxy>{std::span(engine_.sparseTileGroups_[group])};
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
                keysPressed_[keyIndex(event.info.keyPress.key)] = true;
                keysHeld_[keyIndex(event.info.keyPress.key)] = true;
                keysReleased_[keyIndex(event.info.keyPress.key)] = false;

                switch (event.info.keyPress.key) {
                    case app::Key::F11: {
#if !defined(PLATFORM_APPLE)
                        if (window_.getVisibility() != app::WindowVisibility::FULLSCREEN) {
                            window_.setVisibility(app::WindowVisibility::FULLSCREEN);
                        }
                        else {
                            window_.setVisibility(app::WindowVisibility::WINDOWED);
                        }
#endif

                        break;
                    }
                    default:
                        break;
                }
                break;

            case app::WindowEventType::KEY_RELEASED:
                keysPressed_[keyIndex(event.info.keyRelease.key)] = false;
                keysHeld_[keyIndex(event.info.keyRelease.key)] = false;
                keysReleased_[keyIndex(event.info.keyRelease.key)] = true;
                break;

            case app::WindowEventType::MOUSE_MOVED:
                mousePosition_ = event.info.mouseMove.position;
                break;

            default:
                break;
        }
    }
}

void engine::Engine::run() {
    start();

    while (running_) {
        for (std::uint64_t i = 0; i < keysPressed_.size(); i++) {
            keysPressed_[i] = keysPressed_[i] && !keysHeld_[i];
            keysReleased_[i] = keysReleased_[i] && keysHeld_[i];
        }

        manageEvents();

        auto& stagingBufferFence = stagingBufferFences_[renderer_.getFrameCounter().index];

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
    }

    close();
}

std::uint64_t engine::Engine::keyIndex(app::Key key) {
    return static_cast<std::uint64_t>(key);
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

    renderer::DescriptorSetCreateInfo setCreateInfo = {
        .layouts = {descriptorSetLayout_, descriptorSetLayout_},
    };

    descriptorSets_ = renderer::DescriptorPool::allocateDescriptorSets(descriptorPool_, setCreateInfo);

    tilemapDescriptorSet_ = descriptorSets_[0];
    buttonsDescriptorSet_ = descriptorSets_[1];

    renderer::BufferCreateInfo stagingBufferCreateInfo = {
        .device = device,
        .memoryType = renderer::MemoryType::HOST_VISIBLE,
        .usageFlags = renderer::BufferUsageFlags::TRANSFER_SOURCE,
        .sizeBytes = 4 * 1024 * 1024,
    };

    stagingBuffers_.resize(renderer_.getFrameCounter().count);
    stagingBufferFences_.resize(renderer_.getFrameCounter().count);
    stagingBufferSemaphores_.resize(renderer_.getFrameCounter().count);

    for (std::uint64_t i = 0; i < renderer_.getFrameCounter().count; i++) {
        stagingBuffers_[i] = renderer::Buffer::create(stagingBufferCreateInfo);
        stagingBufferFences_[i] = renderer::Fence::create({device, renderer::FenceCreateFlags::START_SIGNALLED});
        stagingBufferSemaphores_[i] = renderer::Semaphore::create(device);
    }

    std::uint64_t stagingBufferOffset = 0;

    auto& transferCommandBuffer = transferCommandBuffers_[renderer_.getFrameCounter().index];
    auto& stagingBuffer = stagingBuffers_[renderer_.getFrameCounter().index];

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

    auto mapping = renderer::Buffer::map(stagingBuffer, totalSize, stagingBufferOffset);

    std::memcpy(mapping.data.data(), tilemapImageData, renderer::Image::getSize(tilemapImage_));
    std::memcpy(mapping.data.data() + renderer::Image::getSize(tilemapImage_), buttonsImageData, renderer::Image::getSize(buttonsImage_));

    renderer::Buffer::unmap(stagingBuffer, mapping);

    renderer::BufferImageCopyRegion tilemapCopyRegion = {
        .bufferOffset = stagingBufferOffset,
        .bufferRowLength = 0,
        .bufferImageHeight = 0,
        .mipLevel = 0,
        .baseArrayLayer = 0,
        .arrayLayerCount = 1,
        .imageAspectMask = renderer::ImageAspectFlags::COLOUR,
        .imageOffset = {0, 0, 0},
        .imageExtent = {static_cast<std::uint32_t>(tilemapWidth), static_cast<std::uint32_t>(tilemapHeight), 1},
    };

    stagingBufferOffset += renderer::Image::getSize(tilemapImage_);

    renderer::BufferImageCopyRegion buttonsCopyRegion = {
        .bufferOffset = stagingBufferOffset,
        .bufferRowLength = 0,
        .bufferImageHeight = 0,
        .mipLevel = 0,
        .baseArrayLayer = 0,
        .arrayLayerCount = 1,
        .imageAspectMask = renderer::ImageAspectFlags::COLOUR,
        .imageOffset = {0, 0, 0},
        .imageExtent = {static_cast<std::uint32_t>(buttonsWidth), static_cast<std::uint32_t>(buttonsHeight), 1},
    };

    stagingBufferOffset += renderer::Image::getSize(buttonsImage_);

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

    currentWorld_ = registry_.create();

    pauseButton_ = registry_.create();

    registry_.emplace<components::Position>(pauseButton_, glm::vec2{0.0f, 0.0f});
    registry_.emplace<components::Scale>(pauseButton_, glm::vec2{1.0f, 1.0f});
    registry_.emplace<components::ButtonTag>(pauseButton_);

    // systems::createButtons(registry_);

    auto& worldComponent = registry_.emplace<components::World>(currentWorld_);

    registry_.emplace<components::TileMesh>(currentWorld_);

    worldComponent.defaultState.path = "assets/worlds/default";

    currentCharacter_ = registry_.create();

    auto& characterInstance = tiles_.emplace_back();
    auto& characterPosition = registry_.emplace<components::Position>(currentCharacter_);
    auto& characterScale = registry_.emplace<components::Scale>(currentCharacter_);

    characterPosition.position = {5.0f, -12.0f};
    characterScale.scale = {0.75f, 1.0f};

    characterInstance = {
        .transform = {
            .position = {characterPosition.position, -0.5f},
            .scale = characterScale.scale,
        },
        .appearance = {
            .texture = {
                .sample = {
                    .position = {0.2, 0.0},
                    .extent = {0.2, 0.2},
                },
                .offset = {0.0, 0.0},
                .repeat = {1.0, 1.0},
            },
            .colourFactor = {1.0f, 1.0f, 1.0f},
        },
    };

    registry_.emplace<components::Speed>(currentCharacter_, 5.0f);
    registry_.emplace<components::Velocity>(currentCharacter_);
    registry_.emplace<components::Acceleration>(currentCharacter_);
    registry_.emplace<components::PositionController>(currentCharacter_, app::Key::W, app::Key::S, app::Key::A, app::Key::D);
    registry_.emplace<components::Proxy<components::TileInstance>>(currentCharacter_, tiles_.size() - 1);
    registry_.emplace<components::CanTriggerTag>(currentCharacter_);
    registry_.emplace<components::CanCollideTag>(currentCharacter_);
    registry_.emplace<components::ActiveCharacterTag>(currentCharacter_);
    registry_.emplace<components::Character>(currentCharacter_);
    registry_.emplace<components::ApplyFrictionTag>(currentCharacter_);
    registry_.emplace<components::Last<components::Position>>(currentCharacter_);
    registry_.emplace<components::Last<components::Velocity>>(currentCharacter_);

    worldTileFirst_ = tiles_.size() - 1;

    systems::loadWorlds(registry_, *this);

    worldTileCount_ = worldComponent.tiles.size() + 1;

    systems::createTileMeshes(registry_, *this, device, transferCommandBuffer, stagingBuffer, stagingBufferOffset);

    currentCamera_ = registry_.create();

    auto& cameraComponent = registry_.emplace<components::Camera>(currentCamera_);
    auto& cameraPosition = registry_.emplace<components::Position>(currentCamera_);
    auto& cameraBuffer = registry_.emplace<components::CameraBuffer>(currentCamera_);
    auto& cameraScale = registry_.emplace<components::Scale>(currentCamera_);

    registry_.emplace<components::ActiveCameraTag>(currentCamera_);
    registry_.emplace<components::Rotation>(currentCamera_);

    cameraComponent.near = -1.0f;
    cameraComponent.far = 1.0f;
    cameraComponent.mode = worldComponent.defaultState.camera.mode;
    cameraComponent.scale = worldComponent.defaultState.camera.scale;

    cameraScale.scale = window_.extent();

    if (worldComponent.defaultState.camera.position.has_value()) {
        cameraPosition.position = worldComponent.defaultState.camera.position.value();
    }
    else {
        cameraPosition.position = {0.0f, 0.0f};
    }

    systems::createCameras(registry_, device);

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
        .buffer = cameraBuffer.buffer,
        .offsetBytes = 0,
        .rangeBytes = renderer::Buffer::size(cameraBuffer.buffer),
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
        .image = tilemapImageView_,
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

void engine::Engine::getDeltaTime() {
    thisFrameTime_ = std::chrono::high_resolution_clock::now();
    deltaTime_ = std::clamp(std::chrono::duration<float>(thisFrameTime_ - lastFrameTime_).count(), 0.0f, 0.1f);
    lastFrameTime_ = thisFrameTime_;
}

void engine::Engine::runPreTransferSystems() {
    // === CAMERA SCALE SET ===
    systems::updateCameraScales(registry_, renderer::Swapchain::getExtent(renderer_.getSwapchain()));

    // === MOVEMENT CONTROLS ===
    systems::updatePositionControllers(registry_, keysHeld_);
    systems::clampSpeeds(registry_);

    // // === UI ===
    // systems::testButtons(registry_, mousePosition_, lastMousePosition_);
    // systems::animateButtons(registry_, deltaTime_);

    // === PHYSICS INTEGRATION ===
    systems::integrateFriction(registry_, registry_.get<components::World>(currentWorld_), deltaTime_);
    systems::integrateMovements(registry_, deltaTime_);

    // === TESTING COLLISIONS ===
    systems::testCollisions(registry_);
    systems::checkTriggers(registry_);

    // === TRIGGER ACTIONS ===
    systems::performTriggers(registry_, *this, api_, deltaTime_);

    // === CAMERA UPDATES ===
    systems::animateCameras(registry_, deltaTime_);
    systems::cameraFollowCharacter(registry_, currentCharacter_, currentCamera_, mousePosition_, deltaTime_);

    // === TRANSFORM TILE INSTANCES
    systems::transformInstances(registry_, tiles_);
}

void engine::Engine::runMidTransferSystems() {
    auto& transferCommandBuffer = transferCommandBuffers_[renderer_.getFrameCounter().index];
    auto& stagingBuffer = stagingBuffers_[renderer_.getFrameCounter().index];

    std::uint64_t stagingBufferOffset = 0;

    // === GPU OPERATIONS ===
    systems::updateCameras(registry_, stagingBuffer, transferCommandBuffer, stagingBufferOffset);
    systems::updateTileMeshes(registry_, *this, transferCommandBuffer, stagingBuffer, stagingBufferOffset);
}

void engine::Engine::runPostTransferSystems() {
    // === CACHE LAST VALUES ===
    systems::cacheLasts<components::Position>(registry_);
    systems::cacheLasts<components::Velocity>(registry_);
    systems::cacheLasts<components::Acceleration>(registry_);
}

void engine::Engine::update() {
    auto& transferCommandBuffer = transferCommandBuffers_[renderer_.getFrameCounter().index];

    auto& stagingBufferFence = stagingBufferFences_[renderer_.getFrameCounter().index];
    auto& stagingBufferSemaphore = stagingBufferSemaphores_[renderer_.getFrameCounter().index];
    auto& transferQueue = renderer_.getTransferQueue();

    renderer::CommandBuffer::reset(transferCommandBuffer);

    getDeltaTime();

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
    auto commandBuffer = renderer_.getCurrentCommandBuffer();
    auto inFlightFence = renderer_.getCurrentInFlightFence();
    auto acquireSemaphore = renderer_.getCurrentAcquireSemaphore();
    auto framebuffer = renderer_.getCurrentFramebuffer();
    auto presentSemaphore = renderer_.getCurrentPresentSemaphore();
    auto& swapchain = renderer_.getSwapchain();
    auto renderPass = renderer_.getRenderPass();
    auto graphicsQueue = renderer_.getGraphicsQueue();

    auto& stagingBufferSemaphore = stagingBufferSemaphores_[renderer_.getFrameCounter().index];

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

    auto& tileMesh = registry_.get<components::TileMesh>(currentWorld_);

    renderer::CommandBuffer::beginCapture(commandBuffer);
    renderer::CommandBuffer::beginRenderPass(commandBuffer, renderPassBeginInfo);

    renderer::CommandBuffer::bindPipeline(commandBuffer, worldPipeline_);
    renderer::CommandBuffer::setPipelineViewports(commandBuffer, {viewport}, 0);
    renderer::CommandBuffer::setPipelineScissors(commandBuffer, {scissor}, 0);

    renderer::CommandBuffer::bindVertexBuffers(commandBuffer, {tileMesh.vertexBuffer, tileMesh.instanceBuffer}, {0, 0}, 0);

    renderer::CommandBuffer::bindDescriptorSets(commandBuffer, renderer::DeviceOperation::GRAPHICS, basicPipelineLayout_, 0, {tilemapDescriptorSet_});
    renderer::CommandBuffer::draw(commandBuffer, 4, static_cast<std::uint32_t>(worldTileCount_), 0, static_cast<std::uint32_t>(worldTileFirst_));

    renderer::CommandBuffer::bindPipeline(commandBuffer, uiPipeline_);
    renderer::CommandBuffer::setPipelineViewports(commandBuffer, {viewport}, 0);
    renderer::CommandBuffer::setPipelineScissors(commandBuffer, {scissor}, 0);

    renderer::CommandBuffer::bindVertexBuffers(commandBuffer, {tileMesh.vertexBuffer, tileMesh.instanceBuffer}, {0, 0}, 0);

    renderer::CommandBuffer::bindDescriptorSets(commandBuffer, renderer::DeviceOperation::GRAPHICS, basicPipelineLayout_, 0, {buttonsDescriptorSet_});
    renderer::CommandBuffer::draw(commandBuffer, 4, static_cast<std::uint32_t>(buttonsTileCount_), 0, static_cast<std::uint32_t>(buttonsTileFirst_));

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

    systems::destroyCameras(registry_);
    systems::destroyTileMeshes(registry_);

    for (std::uint64_t i = 0; i < renderer_.getFrameCounter().count; i++) {
        renderer::Fence::destroy(stagingBufferFences_[i]);
        renderer::Semaphore::destroy(stagingBufferSemaphores_[i]);
        renderer::Buffer::destroy(stagingBuffers_[i]);
    }

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
                    .strideBytes = sizeof(components::TileInstance),
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
                    .strideBytes = sizeof(components::TileInstance),
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