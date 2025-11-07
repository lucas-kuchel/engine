#include <engine/engine.hpp>

#include <components/camera.hpp>
#include <components/entity.hpp>
#include <components/tags.hpp>
#include <components/transforms.hpp>

#include <systems/camera.hpp>
#include <systems/entity.hpp>
#include <systems/transforms.hpp>

#include <cstring>
#include <fstream>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

engine::Engine::Engine()
    : worldGenerator_(*this), stagingManager_(*this) {
}

vulkanite::window::WindowCreateInfo engine::Engine::createWindow() {
    return {
        .subsystem = subsystem_,
        .visibility = vulkanite::window::Visibility::MINIMISED,
        .extent = {1280, 720},
        .title = "Game",
        .resizable = true,
    };
}

void engine::Engine::manageEvents() {
    subsystem_.pollEvents();

    while (window_.hasEvents()) {
        vulkanite::window::Event event = window_.getNextEvent();

        switch (event.type) {
            case vulkanite::window::EventType::CLOSED:
                running_ = false;
                break;

            case vulkanite::window::EventType::KEY_PRESSED:
                inputManager_.updateKeymaps(event.info.keyPress);
                break;

            case vulkanite::window::EventType::KEY_RELEASED:
                inputManager_.updateKeymaps(event.info.keyRelease);
                break;

            case vulkanite::window::EventType::MOUSE_MOVED:
                inputManager_.updateMousePosition(event.info.mouseMove);
                break;

            case vulkanite::window::EventType::MOUSE_SCROLLED:
                inputManager_.updateMouseScroll(event.info.mouseScroll);
                break;

            default:
                break;
        }
    }
}

void engine::Engine::run() {
    stagingManager_.allocate(renderer_.getImageCounter().count, 16 * 1024 * 1024);

    start();

    while (running_) {
        inputManager_.update();

        manageEvents();

        auto& stagingBufferFence = stagingManager_.getCurrentFence();

        renderer_.acquireImage({stagingBufferFence});

        if (renderer_.mustAwaitRestore()) {
            if (window_.getVisibility() == vulkanite::window::Visibility::MINIMISED) {
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
    std::int32_t dummy = 0;

    std::uint8_t* albedoImageData = stbi_load("assets/images/tilemap_albedo.png", &tilemapWidth, &tilemapHeight, &tilemapChannels, 4);
    std::uint8_t* normalImageData = stbi_load("assets/images/tilemap_normal.png", &dummy, &dummy, &dummy, 4);

    for (int i = 0; i < tilemapWidth * tilemapHeight; i++) {
        std::swap(albedoImageData[i * tilemapChannels + 0], albedoImageData[i * tilemapChannels + 2]);
    }

    vulkanite::window::WindowCreateInfo windowCreateInfo = {
        .subsystem = subsystem_,
        .visibility = vulkanite::window::Visibility::WINDOWED,
        .extent = {1280, 720},
        .title = "Game",
        .resizable = true,
    };

    subsystem_.create();
    window_.create(windowCreateInfo);
    renderer_.create(window_);
    worldTileMesh_.create(*this);
    entityTileMesh_.create(*this);
    stagingManager_.allocate(renderer_.getFrameCounter().count, 64 * 1024 * 1024);

    auto& device = renderer_.getDevice();
    auto& transferQueue = renderer_.getTransferQueue();

    vulkanite::renderer::CommandPoolCreateInfo commandPoolCreateInfo = {
        .device = renderer_.getDevice(),
        .queue = renderer_.getTransferQueue(),
    };

    transferCommandPool_.create(commandPoolCreateInfo);

    vulkanite::renderer::DescriptorSetInputInfo bufferInputInfo = {
        .type = vulkanite::renderer::DescriptorInputType::UNIFORM_BUFFER,
        .stageFlags = vulkanite::renderer::DescriptorShaderStageFlags::VERTEX,
        .count = 1,
        .binding = 0,
    };

    vulkanite::renderer::DescriptorSetInputInfo sampler1InputInfo = {
        .type = vulkanite::renderer::DescriptorInputType::IMAGE_SAMPLER,
        .stageFlags = vulkanite::renderer::DescriptorShaderStageFlags::FRAGMENT,
        .count = 1,
        .binding = 1,
    };

    vulkanite::renderer::DescriptorSetInputInfo sampler2InputInfo = {
        .type = vulkanite::renderer::DescriptorInputType::IMAGE_SAMPLER,
        .stageFlags = vulkanite::renderer::DescriptorShaderStageFlags::FRAGMENT,
        .count = 1,
        .binding = 2,
    };

    vulkanite::renderer::DescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = {
        .device = renderer_.getDevice(),
        .inputs = {bufferInputInfo, sampler1InputInfo, sampler2InputInfo},
    };

    descriptorSetLayout_.create(descriptorSetLayoutCreateInfo);

    vulkanite::renderer::DescriptorPoolSize bufferSize = {
        .type = vulkanite::renderer::DescriptorInputType::UNIFORM_BUFFER,
        .count = 1,
    };

    vulkanite::renderer::DescriptorPoolSize imageSize = {
        .type = vulkanite::renderer::DescriptorInputType::IMAGE_SAMPLER,
        .count = 2,
    };

    vulkanite::renderer::DescriptorPoolCreateInfo descriptorPoolCreateInfo = {
        .device = renderer_.getDevice(),
        .poolSizes = {bufferSize, imageSize},
        .maximumSetCount = 1,
    };

    descriptorPool_.create(descriptorPoolCreateInfo);

    vulkanite::renderer::PipelineLayoutCreateInfo pipelineLayoutCreateInfo = {
        .device = renderer_.getDevice(),
        .inputLayouts = {descriptorSetLayout_},
        .pushConstants = {},
    };

    pipelineLayout_.create(pipelineLayoutCreateInfo);

    vulkanite::renderer::ImageCreateInfo albedoImageCreateInfo = {
        .device = renderer_.getDevice(),
        .type = vulkanite::renderer::ImageType::IMAGE_2D,
        .format = vulkanite::renderer::ImageFormat::B8G8R8A8_SRGB,
        .memoryType = vulkanite::renderer::MemoryType::DEVICE_LOCAL,
        .usageFlags = vulkanite::renderer::ImageUsageFlags::SAMPLED | vulkanite::renderer::ImageUsageFlags::TRANSFER_DESTINATION,
        .extent = {320, 320, 1},
        .sampleCount = 1,
        .mipLevels = 1,
        .arrayLayers = 1,
    };

    vulkanite::renderer::ImageCreateInfo normalImageCreateInfo = {
        .device = renderer_.getDevice(),
        .type = vulkanite::renderer::ImageType::IMAGE_2D,
        .format = vulkanite::renderer::ImageFormat::R8G8B8A8_UNORM,
        .memoryType = vulkanite::renderer::MemoryType::DEVICE_LOCAL,
        .usageFlags = vulkanite::renderer::ImageUsageFlags::SAMPLED | vulkanite::renderer::ImageUsageFlags::TRANSFER_DESTINATION,
        .extent = {320, 320, 1},
        .sampleCount = 1,
        .mipLevels = 1,
        .arrayLayers = 1,
    };

    tilemapAlbedoImage_.create(albedoImageCreateInfo);
    tilemapNormalImage_.create(normalImageCreateInfo);

    vulkanite::renderer::ImageViewCreateInfo albedoImageViewCreateInfo = {
        .image = tilemapAlbedoImage_,
        .type = vulkanite::renderer::ImageViewType::IMAGE_2D,
        .aspectFlags = vulkanite::renderer::ImageAspectFlags::COLOUR,
        .baseMipLevel = 0,
        .levelCount = 1,
        .baseArrayLayer = 0,
        .layerCount = 1,
    };

    vulkanite::renderer::ImageViewCreateInfo normalImageViewCreateInfo = {
        .image = tilemapNormalImage_,
        .type = vulkanite::renderer::ImageViewType::IMAGE_2D,
        .aspectFlags = vulkanite::renderer::ImageAspectFlags::COLOUR,
        .baseMipLevel = 0,
        .levelCount = 1,
        .baseArrayLayer = 0,
        .layerCount = 1,
    };

    tilemapAlbedoImageView_.create(albedoImageViewCreateInfo);
    tilemapNormalImageView_.create(normalImageViewCreateInfo);

    vulkanite::renderer::SamplerCreateInfo samplerCreateInfo = {
        .device = renderer_.getDevice(),
        .minFilter = vulkanite::renderer::Filter::NEAREST,
        .magFilter = vulkanite::renderer::Filter::NEAREST,
        .mipmapMode = vulkanite::renderer::MipmapMode::NEAREST,
        .addressModeU = vulkanite::renderer::AddressMode::REPEAT,
        .addressModeV = vulkanite::renderer::AddressMode::REPEAT,
        .addressModeW = vulkanite::renderer::AddressMode::REPEAT,
        .borderColour = vulkanite::renderer::BorderColour::FLOAT_OPAQUE_BLACK,
        .maxAnisotropy = {},
        .comparison = {},
        .unnormalisedCoordinates = false,
        .mipLodBias = 0.0f,
        .minLod = 0.0f,
        .maxLod = 0.0f,
    };

    sampler_.create(samplerCreateInfo);

    vulkanite::renderer::BufferCreateInfo cameraBufferCreateInfo = {
        .device = renderer_.getDevice(),
        .memoryType = vulkanite::renderer::MemoryType::DEVICE_LOCAL,
        .usageFlags = vulkanite::renderer::BufferUsageFlags::UNIFORM | vulkanite::renderer::BufferUsageFlags::TRANSFER_DESTINATION,
        .sizeBytes = sizeof(::components::CameraData),
    };

    cameraBuffer_.create(cameraBufferCreateInfo);

    transferCommandBuffers_ = transferCommandPool_.allocateCommandBuffers(renderer_.getFrameCounter().count);

    vulkanite::renderer::DescriptorSetCreateInfo setCreateInfo = {
        .layouts = {descriptorSetLayout_},
    };

    descriptorSets_ = descriptorPool_.allocateDescriptorSets(setCreateInfo);

    tilemapDescriptorSet_ = descriptorSets_[0];

    auto& transferCommandBuffer = getTransferBuffer();
    auto& stagingBuffer = stagingManager_.getCurrentBuffer();

    vulkanite::renderer::Fence temporaryFence;

    temporaryFence.create({device, 0});

    transferCommandBuffer.beginCapture();

    vulkanite::renderer::ImageMemoryBarrier albedoMemoryBarrier0 = {
        .image = tilemapAlbedoImage_,
        .sourceQueue = {},
        .destinationQueue = {},
        .baseMipLevel = 0,
        .mipLevelCount = 1,
        .baseArrayLayer = 0,
        .arrayLayerCount = 1,
        .oldLayout = vulkanite::renderer::ImageLayout::UNDEFINED,
        .newLayout = vulkanite::renderer::ImageLayout::TRANSFER_DESTINATION_OPTIMAL,
        .aspectMask = vulkanite::renderer::ImageAspectFlags::COLOUR,
        .sourceAccessFlags = vulkanite::renderer::AccessFlags::NONE,
        .destinationAccessFlags = vulkanite::renderer::AccessFlags::TRANSFER_WRITE,
    };

    vulkanite::renderer::ImageMemoryBarrier normalMemoryBarrier0 = {
        .image = tilemapNormalImage_,
        .sourceQueue = {},
        .destinationQueue = {},
        .baseMipLevel = 0,
        .mipLevelCount = 1,
        .baseArrayLayer = 0,
        .arrayLayerCount = 1,
        .oldLayout = vulkanite::renderer::ImageLayout::UNDEFINED,
        .newLayout = vulkanite::renderer::ImageLayout::TRANSFER_DESTINATION_OPTIMAL,
        .aspectMask = vulkanite::renderer::ImageAspectFlags::COLOUR,
        .sourceAccessFlags = vulkanite::renderer::AccessFlags::NONE,
        .destinationAccessFlags = vulkanite::renderer::AccessFlags::TRANSFER_WRITE,
    };

    transferCommandBuffer.pipelineBarrier(vulkanite::renderer::PipelineStageFlags::TOP_OF_PIPE, vulkanite::renderer::PipelineStageFlags::TRANSFER, {albedoMemoryBarrier0, normalMemoryBarrier0});

    std::size_t totalSize = static_cast<std::size_t>(tilemapWidth * tilemapHeight * tilemapChannels) * 2;

    auto mapping = stagingBuffer.map(totalSize, stagingManager_.getOffset());

    std::memcpy(mapping.data.data(), albedoImageData, static_cast<std::size_t>(tilemapWidth * tilemapHeight * tilemapChannels));
    std::memcpy(mapping.data.data() + static_cast<std::size_t>(tilemapWidth * tilemapHeight * tilemapChannels), normalImageData, static_cast<std::size_t>(tilemapWidth * tilemapHeight * tilemapChannels));

    stagingBuffer.unmap(mapping);

    vulkanite::renderer::BufferImageCopyRegion albedoCopyRegion = {
        .bufferOffset = stagingManager_.getOffset(),
        .bufferRowLength = 0,
        .bufferImageHeight = 0,
        .mipLevel = 0,
        .baseArrayLayer = 0,
        .arrayLayerCount = 1,
        .imageOffset = {0, 0, 0},
        .imageExtent = {static_cast<std::uint32_t>(tilemapWidth), static_cast<std::uint32_t>(tilemapHeight), 1},
        .imageAspectMask = vulkanite::renderer::ImageAspectFlags::COLOUR,
    };

    vulkanite::renderer::BufferImageCopyRegion normalCopyRegion = {
        .bufferOffset = stagingManager_.getOffset() + static_cast<std::size_t>(tilemapWidth * tilemapHeight * tilemapChannels),
        .bufferRowLength = 0,
        .bufferImageHeight = 0,
        .mipLevel = 0,
        .baseArrayLayer = 0,
        .arrayLayerCount = 1,
        .imageOffset = {0, 0, 0},
        .imageExtent = {static_cast<std::uint32_t>(tilemapWidth), static_cast<std::uint32_t>(tilemapHeight), 1},
        .imageAspectMask = vulkanite::renderer::ImageAspectFlags::COLOUR,
    };

    stagingManager_.getOffset() += totalSize;

    transferCommandBuffer.copyBufferToImage(stagingBuffer, tilemapAlbedoImage_, vulkanite::renderer::ImageLayout::TRANSFER_DESTINATION_OPTIMAL, {albedoCopyRegion});
    transferCommandBuffer.copyBufferToImage(stagingBuffer, tilemapNormalImage_, vulkanite::renderer::ImageLayout::TRANSFER_DESTINATION_OPTIMAL, {normalCopyRegion});

    vulkanite::renderer::ImageMemoryBarrier albedoMemoryBarrier1 = {
        .image = tilemapAlbedoImage_,
        .sourceQueue = {},
        .destinationQueue = {},
        .baseMipLevel = 0,
        .mipLevelCount = 1,
        .baseArrayLayer = 0,
        .arrayLayerCount = 1,
        .oldLayout = vulkanite::renderer::ImageLayout::TRANSFER_DESTINATION_OPTIMAL,
        .newLayout = vulkanite::renderer::ImageLayout::SHADER_READ_ONLY_OPTIMAL,
        .aspectMask = vulkanite::renderer::ImageAspectFlags::COLOUR,
        .sourceAccessFlags = vulkanite::renderer::AccessFlags::TRANSFER_WRITE,
        .destinationAccessFlags = vulkanite::renderer::AccessFlags::SHADER_READ,
    };

    vulkanite::renderer::ImageMemoryBarrier normalMemoryBarrier1 = {
        .image = tilemapNormalImage_,
        .sourceQueue = {},
        .destinationQueue = {},
        .baseMipLevel = 0,
        .mipLevelCount = 1,
        .baseArrayLayer = 0,
        .arrayLayerCount = 1,
        .oldLayout = vulkanite::renderer::ImageLayout::TRANSFER_DESTINATION_OPTIMAL,
        .newLayout = vulkanite::renderer::ImageLayout::SHADER_READ_ONLY_OPTIMAL,
        .aspectMask = vulkanite::renderer::ImageAspectFlags::COLOUR,
        .sourceAccessFlags = vulkanite::renderer::AccessFlags::TRANSFER_WRITE,
        .destinationAccessFlags = vulkanite::renderer::AccessFlags::SHADER_READ,
    };

    transferCommandBuffer.pipelineBarrier(vulkanite::renderer::PipelineStageFlags::TRANSFER, vulkanite::renderer::PipelineStageFlags::FRAGMENT_SHADER, {albedoMemoryBarrier1, normalMemoryBarrier1});

    using namespace ::components;

    currentEntity_ = registry_.create();

    auto& controller = registry_.emplace<PositionController>(currentEntity_);

    controller.forwardBinding = vulkanite::window::Key::W;
    controller.backwardBinding = vulkanite::window::Key::S;
    controller.leftBinding = vulkanite::window::Key::A;
    controller.rightBinding = vulkanite::window::Key::D;

    auto proxy = entityTilePool_.insert(
        {
            .transform = {
                .position = {0.0, 0.0},
                .scale = {1.0, 1.0},
            },
            .appearance = {
                .texture = {
                    .sample = {
                        .position = {0.1, 0.0},
                        .extent = {0.1, 0.1},
                    },
                    .offset = {0.0, 0.0},
                    .repeat = {1.0, 1.0},
                },
                .colourFactor = {1.0, 1.0, 1.0, 1.0},

            },
        },
        0);

    registry_.emplace<TileProxy>(currentEntity_, proxy);
    registry_.emplace<Position>(currentEntity_, glm::vec3{0.0, 0.0, 0.0});
    registry_.emplace<Acceleration>(currentEntity_);
    registry_.emplace<Velocity>(currentEntity_);
    registry_.emplace<Scale>(currentEntity_, glm::vec2{1.0, 1.0});
    registry_.emplace<TileTag>(currentEntity_);
    registry_.emplace<EntityTag>(currentEntity_);
    registry_.emplace<Speed>(currentEntity_, 5.0);

    ::systems::entities::createEntities(*this);

    std::array<glm::vec2, 4> baseMesh = {
        glm::vec2{1.0, 1.0},
        glm::vec2{0.0, 1.0},
        glm::vec2{1.0, 0.0},
        glm::vec2{0.0, 0.0},
    };

    worldTileMesh_.setBaseMesh(baseMesh);
    entityTileMesh_.setBaseMesh(baseMesh);

    currentCamera_ = registry_.create();

    auto& cameraComponent = registry_.emplace<Camera>(currentCamera_);
    auto& cameraPosition = registry_.emplace<Position>(currentCamera_);
    auto& cameraScale = registry_.emplace<Scale>(currentCamera_);

    registry_.emplace<CameraTarget>(currentCamera_, currentEntity_);
    registry_.emplace<CameraData>(currentCamera_);
    registry_.emplace<CurrentCameraTag>(currentCamera_);

    cameraComponent.near = -1.0f;
    cameraComponent.far = 1.0f;
    cameraComponent.size = 3.0f;
    cameraScale.scale = window_.getExtent();

    ::systems::cameras::calculateCameraData(*this);

    worldGenerator_.setWorldSize({64, 1, 64});
    worldGenerator_.setChunkSize({8, 1, 8});
    worldGenerator_.generate();

    entityTilePool_.sortByDepth();
    entityTileMesh_.createInstanceBuffer(32 * 1024 * 1024);

    cameraPosition.position = {0.0f, 0.0f, 0.0f};

    transferCommandBuffer.endCapture();

    vulkanite::renderer::QueueSubmitInfo submitInfo = {
        .fence = temporaryFence,
        .commandBuffers = {transferCommandBuffer},
        .waits = {},
        .signals = {},
        .waitFlags = {},
    };

    transferQueue.submit(submitInfo);
    device.waitForFences({temporaryFence});

    createBasicPipelineResources();

    vulkanite::renderer::DescriptorSetBufferBinding cameraBufferBinding = {
        .buffer = cameraBuffer_,
        .offsetBytes = 0,
        .rangeBytes = cameraBuffer_.getSize(),
    };

    vulkanite::renderer::DescriptorSetUpdateInfo tilemapUniformBufferUpdateInfo = {
        .set = tilemapDescriptorSet_,
        .inputType = vulkanite::renderer::DescriptorInputType::UNIFORM_BUFFER,
        .binding = 0,
        .arrayElement = 0,
        .buffers = {cameraBufferBinding},
        .images = {},
    };

    descriptorPool_.updateDescriptorSets({tilemapUniformBufferUpdateInfo});

    lastFrameTime_ = std::chrono::high_resolution_clock::now();

    stbi_image_free(albedoImageData);
    stbi_image_free(normalImageData);

    vulkanite::renderer::DescriptorSetImageBinding albedoSamplerBinding = {
        .image = tilemapAlbedoImageView_,
        .sampler = sampler_,
        .layout = vulkanite::renderer::ImageLayout::SHADER_READ_ONLY_OPTIMAL,
    };

    vulkanite::renderer::DescriptorSetUpdateInfo albedoSamplerUpdateInfo = {
        .set = tilemapDescriptorSet_,
        .inputType = vulkanite::renderer::DescriptorInputType::IMAGE_SAMPLER,
        .binding = 1,
        .arrayElement = 0,
        .buffers = {},
        .images = {albedoSamplerBinding},
    };

    vulkanite::renderer::DescriptorSetImageBinding normalSamplerBinding = {
        .image = tilemapNormalImageView_,
        .sampler = sampler_,
        .layout = vulkanite::renderer::ImageLayout::SHADER_READ_ONLY_OPTIMAL,
    };

    vulkanite::renderer::DescriptorSetUpdateInfo normalSamplerUpdateInfo = {
        .set = tilemapDescriptorSet_,
        .inputType = vulkanite::renderer::DescriptorInputType::IMAGE_SAMPLER,
        .binding = 2,
        .arrayElement = 0,
        .buffers = {},
        .images = {normalSamplerBinding},
    };

    descriptorPool_.updateDescriptorSets({albedoSamplerUpdateInfo, normalSamplerUpdateInfo});
    temporaryFence.destroy();
}

void engine::Engine::calculateDeltaTime() {
    thisFrameTime_ = std::chrono::high_resolution_clock::now();
    deltaTime_ = std::clamp(std::chrono::duration<float>(thisFrameTime_ - lastFrameTime_).count(), 0.0f, 0.1f);
    lastFrameTime_ = thisFrameTime_;
}

void engine::Engine::runPreTransferSystems() {
    using namespace ::components;

    auto& cameraComponent = registry_.get<Camera>(currentCamera_);
    cameraComponent.size -= inputManager_.mouseScrollDelta().y;

    cameraComponent.size = glm::clamp(cameraComponent.size, 4.0f, 30.0f);

    ::systems::entities::updateControllers(*this);

    ::systems::integrateMovements(*this);
    ::systems::entities::sortEntities(*this);

    worldGenerator_.generate();
    entityTilePool_.sortByDepth();

    ::systems::cameras::animateCameraPositions(*this);
    ::systems::cameras::animateCameraSizes(*this);
    ::systems::cameras::makeCamerasFollowTarget(*this);
    ::systems::cameras::calculateCameraData(*this);

    ::systems::transformInstances(*this, entityTilePool_);
}

void engine::Engine::runMidTransferSystems() {
    entityTileMesh_.setInstances(entityTilePool_.instances());

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

    transferCommandBuffer.reset();

    calculateDeltaTime();
    runPreTransferSystems();

    transferCommandBuffer.beginCapture();

    runMidTransferSystems();

    transferCommandBuffer.endCapture();

    runPostTransferSystems();

    vulkanite::renderer::QueueSubmitInfo submitInfo = {
        .fence = stagingBufferFence,
        .commandBuffers = {transferCommandBuffer},
        .waits = {},
        .signals = {stagingBufferSemaphore},
        .waitFlags = {},
    };

    transferQueue.submit(submitInfo);
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

    commandBuffer.reset();

    vulkanite::renderer::Viewport viewport = {
        .position = {0.0, 0.0},
        .extent = swapchain.getExtent(),
        .minDepth = 0.0f,
        .maxDepth = 1.0f,
    };

    vulkanite::renderer::Scissor scissor = {
        .offset = {0, 0},
        .extent = swapchain.getExtent(),
    };

    vulkanite::renderer::RenderPassBeginInfo renderPassBeginInfo = {
        .renderPass = renderPass,
        .framebuffer = framebuffer,
        .region = {
            .position = {0, 0},
            .extent = swapchain.getExtent(),
        },
        .colourClearValues = {
            glm::fvec4{0.0, 0.0, 0.0, 1.0},
        },
        .depthClearValue = 1.0f,
        .stencilClearValue = 0xFF,
    };

    using namespace ::components;

    commandBuffer.beginCapture();
    commandBuffer.beginRenderPass(renderPassBeginInfo);

    commandBuffer.bindPipeline(worldPipeline_);
    commandBuffer.setPipelineViewports({viewport}, 0);
    commandBuffer.setPipelineScissors({scissor}, 0);

    // renderer::CommandBuffer::bindVertexBuffers(commandBuffer, {worldTileMesh_.getMeshBuffer(), worldTileMesh_.getInstanceBuffer()}, {0, 0}, 0);
    // renderer::CommandBuffer::draw(commandBuffer, 4, static_cast<std::uint32_t>(worldTilePool_.data().size()), 0, 0);

    commandBuffer.bindDescriptorSets(vulkanite::renderer::DeviceOperation::GRAPHICS, pipelineLayout_, 0, {tilemapDescriptorSet_});
    commandBuffer.bindVertexBuffers({entityTileMesh_.getMeshBuffer(), entityTileMesh_.getInstanceBuffer()}, {0, 0}, 0);
    commandBuffer.draw(4, static_cast<std::uint32_t>(entityTilePool_.data().size()), 0, 0);

    commandBuffer.endRenderPass();
    commandBuffer.endCapture();

    vulkanite::renderer::QueueSubmitInfo submitInfo = {
        .fence = inFlightFence,
        .commandBuffers = {commandBuffer},
        .waits = {stagingBufferSemaphore, acquireSemaphore},
        .signals = {presentSemaphore},
        .waitFlags = {
            vulkanite::renderer::PipelineStageFlags::VERTEX_INPUT,
            vulkanite::renderer::PipelineStageFlags::COLOR_ATTACHMENT_OUTPUT,
        },
    };

    graphicsQueue.submit(submitInfo);
}

void engine::Engine::close() {
    auto& device = renderer_.getDevice();

    device.waitIdle();

    sampler_.destroy();
    tilemapAlbedoImage_.destroy();
    tilemapNormalImage_.destroy();
    tilemapAlbedoImageView_.destroy();
    tilemapNormalImageView_.destroy();
    descriptorPool_.destroy();
    transferCommandPool_.destroy();
    descriptorSetLayout_.destroy();
    pipelineLayout_.destroy();
    worldPipeline_.destroy();
    cameraBuffer_.destroy();
}

void engine::Engine::createBasicPipelineResources() {
    auto& device = renderer_.getDevice();
    auto renderPass = renderer_.getRenderPass();

    std::ifstream tileVertShader("assets/shaders/bin/tile.vert.spv", std::ios::binary | std::ios::ate);
    std::ifstream tileFragShader("assets/shaders/bin/tile.frag.spv", std::ios::binary | std::ios::ate);

    std::uint64_t tileVertShaderSize = static_cast<std::uint64_t>(tileVertShader.tellg());
    std::uint64_t tileFragShaderSize = static_cast<std::uint64_t>(tileFragShader.tellg());

    tileVertShader.seekg(0, std::ios::beg);
    tileFragShader.seekg(0, std::ios::beg);

    std::vector<std::uint32_t> tileVertShaderBinary(tileVertShaderSize / sizeof(std::uint32_t));
    std::vector<std::uint32_t> tileFragShaderBinary(tileFragShaderSize / sizeof(std::uint32_t));

    tileVertShader.read(reinterpret_cast<char*>(tileVertShaderBinary.data()), static_cast<std::uint32_t>(tileVertShaderSize));
    tileFragShader.read(reinterpret_cast<char*>(tileFragShaderBinary.data()), static_cast<std::uint32_t>(tileFragShaderSize));

    vulkanite::renderer::ShaderModuleCreateInfo tileVertShaderModuleCreateInfo = {
        .device = device,
        .data = tileVertShaderBinary,
    };

    vulkanite::renderer::ShaderModuleCreateInfo tileFragShaderModuleCreateInfo = {
        .device = device,
        .data = tileFragShaderBinary,
    };

    vulkanite::renderer::ShaderModule tileVertShaderModule;
    vulkanite::renderer::ShaderModule tileFragShaderModule;

    tileVertShaderModule.create(tileVertShaderModuleCreateInfo);
    tileFragShaderModule.create(tileFragShaderModuleCreateInfo);

    vulkanite::renderer::PipelineCreateInfo worldPipelineCreateInfo = {
        .renderPass = renderPass,
        .layout = pipelineLayout_,
        .shaderStages = {
            vulkanite::renderer::ShaderStageInfo{
                tileVertShaderModule,
                vulkanite::renderer::ShaderStage::VERTEX,
            },
            vulkanite::renderer::ShaderStageInfo{
                tileFragShaderModule,
                vulkanite::renderer::ShaderStage::FRAGMENT,
            },
        },
        .subpassIndex = 0,
        .viewportCount = 1,
        .scissorCount = 1,
        .vertexInput = {
            .bindings = {
                vulkanite::renderer::VertexInputBindingDescription{
                    .inputRate = vulkanite::renderer::VertexInputRate::PER_VERTEX,
                    .binding = 0,
                    .strideBytes = sizeof(glm::vec2),
                },
                vulkanite::renderer::VertexInputBindingDescription{
                    .inputRate = vulkanite::renderer::VertexInputRate::PER_INSTANCE,
                    .binding = 1,
                    .strideBytes = sizeof(TileInstance),
                },
            },
            .attributes = {
                // === VERTEX ===
                vulkanite::renderer::VertexAttributeDescription{
                    .format = vulkanite::renderer::VertexAttributeFormat::R32G32_FLOAT,
                    .binding = 0,
                    .location = 0,
                },
                // === POSITION ===
                vulkanite::renderer::VertexAttributeDescription{
                    .format = vulkanite::renderer::VertexAttributeFormat::R32G32_FLOAT,
                    .binding = 1,
                    .location = 1,
                },
                // === SCALE ===
                vulkanite::renderer::VertexAttributeDescription{
                    .format = vulkanite::renderer::VertexAttributeFormat::R32G32_FLOAT,
                    .binding = 1,
                    .location = 2,
                },
                // === TEXTURE POSITION ===
                vulkanite::renderer::VertexAttributeDescription{
                    .format = vulkanite::renderer::VertexAttributeFormat::R32G32_FLOAT,
                    .binding = 1,
                    .location = 3,
                },
                // === TEXTURE EXTENT ===
                vulkanite::renderer::VertexAttributeDescription{
                    .format = vulkanite::renderer::VertexAttributeFormat::R32G32_FLOAT,
                    .binding = 1,
                    .location = 4,
                },
                // === TEXTURE OFFSET ===
                vulkanite::renderer::VertexAttributeDescription{
                    .format = vulkanite::renderer::VertexAttributeFormat::R32G32_FLOAT,
                    .binding = 1,
                    .location = 5,
                },
                // === TEXTURE REPEAT ===
                vulkanite::renderer::VertexAttributeDescription{
                    .format = vulkanite::renderer::VertexAttributeFormat::R32G32_FLOAT,
                    .binding = 1,
                    .location = 6,
                },
                // === COLOUR FACTOR ===
                vulkanite::renderer::VertexAttributeDescription{
                    .format = vulkanite::renderer::VertexAttributeFormat::R32G32B32A32_FLOAT,
                    .binding = 1,
                    .location = 7,
                },
            },
        },
        .inputAssembly = {
            .topology = vulkanite::renderer::PolygonTopology::TRIANGLE_STRIP,
            .primitiveRestart = false,
        },
        .rasterisation = {
            .frontFaceWinding = vulkanite::renderer::PolygonFaceWinding::ANTICLOCKWISE,
            .cullMode = vulkanite::renderer::PolygonCullMode::NEVER,
            .frontface = {
                .depthComparison = vulkanite::renderer::CompareOperation::LESS_EQUAL,
                .stencilComparison = vulkanite::renderer::CompareOperation::ALWAYS,
                .stencilFailOperation = vulkanite::renderer::ValueOperation::KEEP,
                .depthFailOperation = vulkanite::renderer::ValueOperation::KEEP,
                .passOperation = vulkanite::renderer::ValueOperation::KEEP,
                .stencilCompareMask = 0xFF,
                .stencilWriteMask = 0xFF,
            },
            .backface = {
                .depthComparison = vulkanite::renderer::CompareOperation::LESS_EQUAL,
                .stencilComparison = vulkanite::renderer::CompareOperation::ALWAYS,
                .stencilFailOperation = vulkanite::renderer::ValueOperation::KEEP,
                .depthFailOperation = vulkanite::renderer::ValueOperation::KEEP,
                .passOperation = vulkanite::renderer::ValueOperation::KEEP,
                .stencilCompareMask = 0xFF,
                .stencilWriteMask = 0xFF,
            },
            .depthClampEnable = false,
            .depthTestEnable = false,
            .depthWriteEnable = false,
            .depthBoundsTestEnable = false,
            .stencilTestEnable = false,
        },
        .multisample = {},
        .colourBlend = {
            .attachments = {vulkanite::renderer::ColourBlendAttachment{}},
        },
    };

    pipelines_ = device.createPipelines({worldPipelineCreateInfo});

    worldPipeline_ = pipelines_[0];

    tileVertShaderModule.destroy();
    tileFragShaderModule.destroy();
}