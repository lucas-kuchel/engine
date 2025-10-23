#include <engine/engine.hpp>

#include <engine/components/defaults.hpp>
#include <engine/components/entity_tags.hpp>
#include <engine/components/space.hpp>

#include <engine/components/proxy.hpp>

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
        "getTileInstances", &EngineAPI::getTileInstances);

    luaState_["engine"] = &api_;

    luaState_.new_usertype<TileProxy>(
        "TileProxy",
        "index", &TileProxy::index);

    luaState_.new_usertype<TileInstance::Texture::Sample>(
        "TileInstanceTextureSample",
        "position", &TileInstance::Texture::Sample::position,
        "extent", &TileInstance::Texture::Sample::extent);

    luaState_.new_usertype<TileInstance::Texture>(
        "TileInstanceTexture",
        "sample", &TileInstance::Texture::sample,
        "scale", &TileInstance::Texture::scale,
        "offset", &TileInstance::Texture::offset);

    luaState_.new_usertype<TileInstance>(
        "TileInstance",
        "position", &TileInstance::position,
        "scale", &TileInstance::scale,
        "texture", &TileInstance::texture,
        "colourMultiplier", &TileInstance::colourMultiplier);

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
}

engine::Engine::~Engine() {
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

        world.currentSpace = spaceEntity;
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

    camera.mode = world.defaults.camera.mode;

    // TODO: add flag to space for camera transitions
    if (true) {
        auto& cameraScaleAnimator = engine_.registry_.emplace_or_replace<components::CameraScaleAnimator>(engine_.currentCamera_);
        cameraScaleAnimator.timeElapsed = 0.0f;
        cameraScaleAnimator.duration = 1.0f;
        cameraScaleAnimator.targetScale = world.defaults.camera.scale;
        cameraScaleAnimator.startScale = camera.scale;

        if (world.defaults.camera.position.has_value()) {
            auto& cameraPositionAnimator = engine_.registry_.emplace_or_replace<components::CameraPositionAnimator>(engine_.currentCamera_);

            cameraPositionAnimator.timeElapsed = 0.0f;
            cameraPositionAnimator.duration = 1.0f;
            cameraPositionAnimator.startPosition = position.position;
            cameraPositionAnimator.targetPosition = world.defaults.camera.position.value();
        }
    }
    else {
        camera.scale = world.defaults.camera.scale;

        if (world.defaults.camera.position.has_value()) {
            position.position = world.defaults.camera.position.value();
        }
    }

    world.currentSpace = entt::null;
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
    std::int32_t width = 0;
    std::int32_t height = 0;
    std::int32_t channels = 0;

    std::uint8_t* tilemapImageData = stbi_load("assets/images/tilemap.png", &width, &height, &channels, 4);

    for (int i = 0; i < width * height; i++) {
        std::swap(tilemapImageData[i * 4 + 0], tilemapImageData[i * 4 + 2]);
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
        .extent = {static_cast<std::uint32_t>(width), static_cast<std::uint32_t>(height), 1},
        .sampleCount = 1,
        .mipLevels = 1,
        .arrayLayers = 1,
    };

    tilemapImage_ = renderer::Image::create(tilemapImageCreateInfo);

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
        .count = 1,
    };

    renderer::DescriptorPoolSize imageSize = {
        .type = renderer::DescriptorInputType::IMAGE_SAMPLER,
        .count = 1,
    };

    renderer::DescriptorPoolCreateInfo poolCreateInfo = {
        .device = device,
        .poolSizes = {bufferSize, imageSize},
        .maximumSetCount = 1,
    };

    descriptorPool_ = renderer::DescriptorPool::create(poolCreateInfo);

    renderer::DescriptorSetCreateInfo setCreateInfo = {
        .layouts = {descriptorSetLayout_},
    };

    descriptorSets_ = renderer::DescriptorPool::allocateDescriptorSets(descriptorPool_, setCreateInfo);
    basicDescriptorSet_ = descriptorSets_[0];

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

    renderer::ImageMemoryBarrier memoryBarrier0 = {
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

    renderer::CommandBuffer::pipelineBarrier(transferCommandBuffer, renderer::PipelineStageFlags::TOP_OF_PIPE, renderer::PipelineStageFlags::TRANSFER, {memoryBarrier0});

    auto mapping = renderer::Buffer::map(stagingBuffer, renderer::Image::getSize(tilemapImage_), stagingBufferOffset);

    std::memcpy(mapping.data.data(), tilemapImageData, renderer::Image::getSize(tilemapImage_));

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
        .imageExtent = {static_cast<std::uint32_t>(width), static_cast<std::uint32_t>(height), 1},
    };

    stagingBufferOffset += renderer::Image::getSize(tilemapImage_);

    renderer::CommandBuffer::copyBufferToImage(transferCommandBuffer, stagingBuffer, tilemapImage_, renderer::ImageLayout::TRANSFER_DESTINATION_OPTIMAL, {tilemapCopyRegion});

    renderer::ImageMemoryBarrier memoryBarrier1 = {
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

    renderer::CommandBuffer::pipelineBarrier(transferCommandBuffer, renderer::PipelineStageFlags::TRANSFER, renderer::PipelineStageFlags::FRAGMENT_SHADER, {memoryBarrier1});

    currentWorld_ = registry_.create();

    auto& worldComponent = registry_.emplace<components::World>(currentWorld_);

    registry_.emplace<components::TileMesh>(currentWorld_);

    worldComponent.path = "assets/worlds/default";

    systems::loadWorlds(registry_, *this);

    currentCharacter_ = registry_.create();

    auto& characterInstance = tiles_.emplace_back();
    auto& characterPosition = registry_.emplace<components::Position>(currentCharacter_);

    characterPosition.position = {0.0f, 0.0f};
    characterPosition.depth = -0.5f;
    characterInstance.texture = {
        .sample = {
            .position = {0.2, 0.0},
            .extent = {0.2, 0.2},
        },
        .offset = {0.0, 0.0},
        .scale = {1.0, 1.0},
    };
    characterInstance.colourMultiplier = {1.0f, 1.0f, 1.0f};

    registry_.emplace<components::Speed>(currentCharacter_, 5.0f);
    registry_.emplace<components::Scale>(currentCharacter_);
    registry_.emplace<components::Velocity>(currentCharacter_);
    registry_.emplace<components::Acceleration>(currentCharacter_);
    registry_.emplace<components::PositionController>(currentCharacter_, app::Key::W, app::Key::S, app::Key::A, app::Key::D);
    registry_.emplace<components::Proxy<components::TileInstance>>(currentCharacter_, tiles_.size() - 1);
    registry_.emplace<components::CanTriggerTag>(currentCharacter_);
    registry_.emplace<components::CanCollideTag>(currentCharacter_);
    registry_.emplace<components::ActiveCharacterTag>(currentCharacter_);
    registry_.emplace<components::Character>(currentCharacter_);

    systems::createTileMeshes(registry_, *this, device, transferCommandBuffer, stagingBuffer, stagingBufferOffset);

    currentCamera_ = registry_.create();

    auto& cameraComponent = registry_.emplace<components::Camera>(currentCamera_);
    auto& cameraPosition = registry_.emplace<components::Position>(currentCamera_);
    auto& cameraBuffer = registry_.emplace<components::CameraBuffer>(currentCamera_);
    auto& cameraScale = registry_.emplace<components::Scale>(currentCamera_);

    registry_.emplace<components::ActiveCameraTag>(currentCamera_);

    cameraComponent.near = -1.0f;
    cameraComponent.far = 1.0f;
    cameraComponent.mode = worldComponent.defaults.camera.mode;
    cameraComponent.scale = worldComponent.defaults.camera.scale;
    cameraPosition.depth = 0.0f;

    cameraScale.scale = window_.extent();

    if (worldComponent.defaults.camera.position.has_value()) {
        cameraPosition.position = glm::vec3(worldComponent.defaults.camera.position.value(), 0.5f);
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

    renderer::DescriptorSetUpdateInfo uniformBufferUpdateInfo = {
        .set = basicDescriptorSet_,
        .inputType = renderer::DescriptorInputType::UNIFORM_BUFFER,
        .binding = 0,
        .arrayElement = 0,
        .buffers = {cameraBufferBinding},
        .images = {},
    };

    renderer::DescriptorPool::updateDescriptorSets(descriptorPool_, {uniformBufferUpdateInfo});

    renderer::ImageViewCreateInfo tilemapImageViewCreateInfo = {
        .image = tilemapImage_,
        .type = renderer::ImageViewType::IMAGE_2D,
        .aspectFlags = renderer::ImageAspectFlags::COLOUR,
        .baseMipLevel = 0,
        .levelCount = 1,
        .baseArrayLayer = 0,
        .layerCount = 1,
    };

    tilemapImageView_ = renderer::ImageView::create(tilemapImageViewCreateInfo);

    renderer::DescriptorSetImageBinding samplerBinding = {
        .image = tilemapImageView_,
        .sampler = sampler_,
        .layout = renderer::ImageLayout::SHADER_READ_ONLY_OPTIMAL,
    };

    renderer::DescriptorSetUpdateInfo samplerUpdateInfo = {
        .set = basicDescriptorSet_,
        .inputType = renderer::DescriptorInputType::IMAGE_SAMPLER,
        .binding = 1,
        .arrayElement = 0,
        .buffers = {},
        .images = {samplerBinding},
    };

    renderer::DescriptorPool::updateDescriptorSets(descriptorPool_, {samplerUpdateInfo});

    lastFrameTime_ = std::chrono::high_resolution_clock::now();

    stbi_image_free(tilemapImageData);
}

void engine::Engine::update() {
    auto& transferCommandBuffer = transferCommandBuffers_[renderer_.getFrameCounter().index];
    auto& stagingBuffer = stagingBuffers_[renderer_.getFrameCounter().index];
    auto& stagingBufferFence = stagingBufferFences_[renderer_.getFrameCounter().index];
    auto& stagingBufferSemaphore = stagingBufferSemaphores_[renderer_.getFrameCounter().index];

    auto transferQueue = renderer_.getTransferQueue();

    renderer::CommandBuffer::reset(transferCommandBuffer);

    std::uint64_t stagingBufferOffset = 0;

    thisFrameTime_ = std::chrono::high_resolution_clock::now();
    deltaTime_ = std::clamp(std::chrono::duration<float>(thisFrameTime_ - lastFrameTime_).count(), 0.0f, 0.1f);
    lastFrameTime_ = thisFrameTime_;

    renderer::CommandBuffer::beginCapture(transferCommandBuffer);

    systems::cachePositions(registry_);
    systems::updateCameraScale(registry_, renderer::Swapchain::getExtent(renderer_.getSwapchain()));
    systems::updatePositionControllers(registry_, keysHeld_);
    systems::integrateMovements(registry_, deltaTime_);
    systems::testCollisions(registry_);
    systems::checkTriggers(registry_);
    systems::performTriggers(registry_, *this, api_, deltaTime_);
    systems::animateCameras(registry_, deltaTime_);
    systems::cameraFollowCharacter(registry_, currentCharacter_, currentCamera_, deltaTime_);
    systems::transformInstances(registry_, tiles_);
    systems::updateCameras(registry_, stagingBuffer, transferCommandBuffer, stagingBufferOffset);
    systems::updateTileMeshes(registry_, *this, transferCommandBuffer, stagingBuffer, stagingBufferOffset);

    renderer::CommandBuffer::endCapture(transferCommandBuffer);

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

    renderer::CommandBuffer::beginCapture(commandBuffer);
    renderer::CommandBuffer::beginRenderPass(commandBuffer, renderPassBeginInfo);
    renderer::CommandBuffer::bindPipeline(commandBuffer, basicPipeline_);
    renderer::CommandBuffer::setPipelineViewports(commandBuffer, {viewport}, 0);
    renderer::CommandBuffer::setPipelineScissors(commandBuffer, {scissor}, 0);
    renderer::CommandBuffer::bindDescriptorSets(commandBuffer, renderer::DeviceOperation::GRAPHICS, basicPipelineLayout_, 0, {basicDescriptorSet_});

    auto& tileMesh = registry_.get<components::TileMesh>(currentWorld_);

    renderer::CommandBuffer::bindVertexBuffers(commandBuffer, {tileMesh.vertexBuffer, tileMesh.instanceBuffer}, {0, 0}, 0);
    renderer::CommandBuffer::draw(commandBuffer, 4, static_cast<std::uint32_t>(tiles_.size()), 0, 0);

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
    renderer::Pipeline::destroy(basicPipeline_);
    renderer::PipelineLayout::destroy(basicPipelineLayout_);
    renderer::ImageView::destroy(tilemapImageView_);
    renderer::Image::destroy(tilemapImage_);
    renderer::Sampler::destroy(sampler_);
}

void engine::Engine::createBasicPipelineResources() {
    auto& device = renderer_.getDevice();
    auto renderPass = renderer_.getRenderPass();

    std::ifstream vertexShader("assets/shaders/basic.vert.spv", std::ios::binary | std::ios::ate);
    std::ifstream fragmentShader("assets/shaders/basic.frag.spv", std::ios::binary | std::ios::ate);

    std::uint64_t vertexShaderSize = static_cast<std::uint64_t>(vertexShader.tellg());
    std::uint64_t fragmentShaderSize = static_cast<std::uint64_t>(fragmentShader.tellg());

    vertexShader.seekg(0, std::ios::beg);
    fragmentShader.seekg(0, std::ios::beg);

    std::vector<std::uint32_t> vertexShaderBinary(vertexShaderSize / sizeof(std::uint32_t));
    std::vector<std::uint32_t> fragmentShaderBinary(fragmentShaderSize / sizeof(std::uint32_t));

    vertexShader.read(reinterpret_cast<char*>(vertexShaderBinary.data()), static_cast<std::uint32_t>(vertexShaderSize));
    fragmentShader.read(reinterpret_cast<char*>(fragmentShaderBinary.data()), static_cast<std::uint32_t>(fragmentShaderSize));

    renderer::ShaderModuleCreateInfo vertexShaderModuleCreateInfo = {
        .device = device,
        .data = vertexShaderBinary,
    };

    renderer::ShaderModuleCreateInfo fragmentShaderModuleCreateInfo = {
        .device = device,
        .data = fragmentShaderBinary,
    };

    renderer::ShaderModule vertexShaderModule = renderer::ShaderModule::create(vertexShaderModuleCreateInfo);
    renderer::ShaderModule fragmentShaderModule = renderer::ShaderModule::create(fragmentShaderModuleCreateInfo);

    renderer::PipelineLayoutCreateInfo pipelineLayoutCreateInfo = {
        .device = device,
        .inputLayouts = {descriptorSetLayout_},
        .pushConstants = {},
    };

    basicPipelineLayout_ = renderer::PipelineLayout::create(pipelineLayoutCreateInfo);

    renderer::PipelineCreateInfo pipelineCreateInfo = {
        .renderPass = renderPass,
        .layout = basicPipelineLayout_,
        .subpassIndex = 0,
        .shaderStages = {
            {
                vertexShaderModule,
                renderer::ShaderStage::VERTEX,
                "main",
            },
            {
                fragmentShaderModule,
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
                renderer::VertexAttributeDescription{
                    .format = renderer::VertexAttributeFormat::R32G32_FLOAT,
                    .binding = 0,
                    .location = 0,
                },
                renderer::VertexAttributeDescription{
                    .format = renderer::VertexAttributeFormat::R32G32_FLOAT,
                    .binding = 1,
                    .location = 1,
                },
                renderer::VertexAttributeDescription{
                    .format = renderer::VertexAttributeFormat::R32G32_FLOAT,
                    .binding = 1,
                    .location = 2,
                },
                renderer::VertexAttributeDescription{
                    .format = renderer::VertexAttributeFormat::R32G32_FLOAT,
                    .binding = 1,
                    .location = 3,
                },
                renderer::VertexAttributeDescription{
                    .format = renderer::VertexAttributeFormat::R32G32_FLOAT,
                    .binding = 1,
                    .location = 4,
                },
                renderer::VertexAttributeDescription{
                    .format = renderer::VertexAttributeFormat::R32G32_FLOAT,
                    .binding = 1,
                    .location = 5,
                },
                renderer::VertexAttributeDescription{
                    .format = renderer::VertexAttributeFormat::R32G32B32_FLOAT,
                    .binding = 1,
                    .location = 6,
                },
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

    pipelines_ = renderer::Device::createPipelines(device, {pipelineCreateInfo});

    basicPipeline_ = pipelines_.front();

    renderer::ShaderModule::destroy(vertexShaderModule);
    renderer::ShaderModule::destroy(fragmentShaderModule);
}