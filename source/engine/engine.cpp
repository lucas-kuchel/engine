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
    : worldGenerator_(*this), window_(createWindow()), renderer_(window_), stagingManager_(*this), worldTileMesh_(*this), entityTileMesh_(*this) {
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
    stagingManager_.allocate(renderer_.getImageCounter().count, 16 * 1024 * 1024);

    start();

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
    std::int32_t dummy = 0;

    std::uint8_t* albedoImageData = stbi_load("assets/images/tilemap_albedo.png", &tilemapWidth, &tilemapHeight, &tilemapChannels, 4);
    std::uint8_t* normalImageData = stbi_load("assets/images/tilemap_normal.png", &dummy, &dummy, &dummy, 4);

    for (int i = 0; i < tilemapWidth * tilemapHeight; i++) {
        std::swap(albedoImageData[i * tilemapChannels + 0], albedoImageData[i * tilemapChannels + 2]);
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

    renderer::ImageCreateInfo albedoImageCreateInfo = {
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

    renderer::ImageCreateInfo normalImageCreateInfo = {
        .device = device,
        .type = renderer::ImageType::IMAGE_2D,
        .format = renderer::ImageFormat::R8G8B8A8_UNORM,
        .memoryType = renderer::MemoryType::DEVICE_LOCAL,
        .usageFlags = renderer::ImageUsageFlags::SAMPLED | renderer::ImageUsageFlags::TRANSFER_DESTINATION,
        .extent = {static_cast<std::uint32_t>(tilemapWidth), static_cast<std::uint32_t>(tilemapHeight), 1},
        .sampleCount = 1,
        .mipLevels = 1,
        .arrayLayers = 1,
    };

    tilemapAlbedoImage_ = renderer::Image::create(albedoImageCreateInfo);
    tilemapNormalImage_ = renderer::Image::create(normalImageCreateInfo);

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

    renderer::DescriptorSetInputInfo sampler1InputInfo = {
        .type = renderer::DescriptorInputType::IMAGE_SAMPLER,
        .stageFlags = renderer::DescriptorShaderStageFlags::FRAGMENT,
        .count = 1,
        .binding = 1,
    };

    renderer::DescriptorSetInputInfo sampler2InputInfo = {
        .type = renderer::DescriptorInputType::IMAGE_SAMPLER,
        .stageFlags = renderer::DescriptorShaderStageFlags::FRAGMENT,
        .count = 1,
        .binding = 2,
    };

    renderer::DescriptorSetLayoutCreateInfo layoutCreateInfo = {
        .device = device,
        .inputs = {bufferInputInfo, sampler1InputInfo, sampler2InputInfo},
    };

    descriptorSetLayout_ = renderer::DescriptorSetLayout::create(layoutCreateInfo);

    renderer::DescriptorPoolSize bufferSize = {
        .type = renderer::DescriptorInputType::UNIFORM_BUFFER,
        .count = 1,
    };

    renderer::DescriptorPoolSize imageSize = {
        .type = renderer::DescriptorInputType::IMAGE_SAMPLER,
        .count = 2,
    };

    renderer::DescriptorPoolCreateInfo poolCreateInfo = {
        .device = device,
        .poolSizes = {bufferSize, imageSize},
        .maximumSetCount = 1,
    };

    descriptorPool_ = renderer::DescriptorPool::create(poolCreateInfo);

    renderer::BufferCreateInfo cameraBufferCreateInfo = {
        .device = device,
        .memoryType = renderer::MemoryType::DEVICE_LOCAL,
        .usageFlags = renderer::BufferUsageFlags::UNIFORM | renderer::BufferUsageFlags::TRANSFER_DESTINATION,
        .sizeBytes = sizeof(::components::CameraData),
    };

    cameraBuffer_ = renderer::Buffer::create(cameraBufferCreateInfo);

    renderer::DescriptorSetCreateInfo setCreateInfo = {
        .layouts = {descriptorSetLayout_},
    };

    descriptorSets_ = renderer::DescriptorPool::allocateDescriptorSets(descriptorPool_, setCreateInfo);

    tilemapDescriptorSet_ = descriptorSets_[0];

    auto& transferCommandBuffer = getTransferBuffer();
    auto& stagingBuffer = stagingManager_.getCurrentBuffer();

    renderer::Fence temporaryFence = renderer::Fence::create({device, 0});
    renderer::CommandBuffer::beginCapture(transferCommandBuffer);

    renderer::ImageMemoryBarrier albedoMemoryBarrier0 = {
        .image = tilemapAlbedoImage_,
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

    renderer::ImageMemoryBarrier normalMemoryBarrier0 = {
        .image = tilemapNormalImage_,
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

    renderer::CommandBuffer::pipelineBarrier(transferCommandBuffer, renderer::PipelineStageFlags::TOP_OF_PIPE, renderer::PipelineStageFlags::TRANSFER, {albedoMemoryBarrier0, normalMemoryBarrier0});

    std::size_t totalSize = static_cast<std::size_t>(tilemapWidth * tilemapHeight * tilemapChannels) * 2;

    auto mapping = renderer::Buffer::map(stagingBuffer, totalSize, stagingManager_.getOffset());

    std::memcpy(mapping.data.data(), albedoImageData, totalSize / 2);
    std::memcpy(mapping.data.data() + totalSize / 2, normalImageData, totalSize / 2);

    renderer::Buffer::unmap(stagingBuffer, mapping);

    renderer::BufferImageCopyRegion albedoCopyRegion = {
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

    renderer::BufferImageCopyRegion normalCopyRegion = {
        .bufferOffset = stagingManager_.getOffset() + totalSize / 2,
        .bufferRowLength = 0,
        .bufferImageHeight = 0,
        .mipLevel = 0,
        .baseArrayLayer = 0,
        .arrayLayerCount = 1,
        .imageAspectMask = renderer::ImageAspectFlags::COLOUR,
        .imageOffset = {0, 0, 0},
        .imageExtent = {static_cast<std::uint32_t>(tilemapWidth), static_cast<std::uint32_t>(tilemapHeight), 1},
    };

    stagingManager_.getOffset() += totalSize / 2;

    renderer::CommandBuffer::copyBufferToImage(transferCommandBuffer, stagingBuffer, tilemapAlbedoImage_, renderer::ImageLayout::TRANSFER_DESTINATION_OPTIMAL, {albedoCopyRegion});
    renderer::CommandBuffer::copyBufferToImage(transferCommandBuffer, stagingBuffer, tilemapNormalImage_, renderer::ImageLayout::TRANSFER_DESTINATION_OPTIMAL, {normalCopyRegion});

    renderer::ImageMemoryBarrier albedoMemoryBarrier1 = {
        .image = tilemapAlbedoImage_,
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

    renderer::ImageMemoryBarrier normalMemoryBarrier1 = {
        .image = tilemapNormalImage_,
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

    renderer::CommandBuffer::pipelineBarrier(transferCommandBuffer, renderer::PipelineStageFlags::TRANSFER, renderer::PipelineStageFlags::FRAGMENT_SHADER, {albedoMemoryBarrier1, normalMemoryBarrier1});

    using namespace ::components;

    currentEntity_ = registry_.create();

    auto& controller = registry_.emplace<PositionController>(currentEntity_);

    controller.forwardBinding = app::Key::W;
    controller.backwardBinding = app::Key::S;
    controller.leftBinding = app::Key::A;
    controller.rightBinding = app::Key::D;

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
    cameraScale.scale = window_.extent();

    ::systems::cameras::calculateCameraData(*this);

    worldGenerator_.setWorldSize({16, 1, 16});
    worldGenerator_.setChunkSize({8, 1, 8});
    worldGenerator_.generate();

    entityTilePool_.sortByDepth();
    entityTileMesh_.createInstanceBuffer(32 * 1024 * 1024);

    cameraPosition.position = {0.0f, 0.0f, 0.0f};

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

    renderer::DescriptorPool::updateDescriptorSets(descriptorPool_, {tilemapUniformBufferUpdateInfo});

    renderer::ImageViewCreateInfo albedoImageViewCreateInfo = {
        .image = tilemapAlbedoImage_,
        .type = renderer::ImageViewType::IMAGE_2D,
        .aspectFlags = renderer::ImageAspectFlags::COLOUR,
        .baseMipLevel = 0,
        .levelCount = 1,
        .baseArrayLayer = 0,
        .layerCount = 1,
    };

    renderer::ImageViewCreateInfo normalImageViewCreateInfo = {
        .image = tilemapNormalImage_,
        .type = renderer::ImageViewType::IMAGE_2D,
        .aspectFlags = renderer::ImageAspectFlags::COLOUR,
        .baseMipLevel = 0,
        .levelCount = 1,
        .baseArrayLayer = 0,
        .layerCount = 1,
    };

    tilemapAlbedoImageView_ = renderer::ImageView::create(albedoImageViewCreateInfo);
    tilemapNormalImageView_ = renderer::ImageView::create(normalImageViewCreateInfo);

    lastFrameTime_ = std::chrono::high_resolution_clock::now();

    stbi_image_free(albedoImageData);

    renderer::DescriptorSetImageBinding albedoSamplerBinding = {
        .image = tilemapAlbedoImageView_,
        .sampler = sampler_,
        .layout = renderer::ImageLayout::SHADER_READ_ONLY_OPTIMAL,
    };

    renderer::DescriptorSetUpdateInfo albedoSamplerUpdateInfo = {
        .set = tilemapDescriptorSet_,
        .inputType = renderer::DescriptorInputType::IMAGE_SAMPLER,
        .binding = 1,
        .arrayElement = 0,
        .buffers = {},
        .images = {albedoSamplerBinding},
    };

    renderer::DescriptorSetImageBinding normalSamplerBinding = {
        .image = tilemapNormalImageView_,
        .sampler = sampler_,
        .layout = renderer::ImageLayout::SHADER_READ_ONLY_OPTIMAL,
    };

    renderer::DescriptorSetUpdateInfo normalSamplerUpdateInfo = {
        .set = tilemapDescriptorSet_,
        .inputType = renderer::DescriptorInputType::IMAGE_SAMPLER,
        .binding = 2,
        .arrayElement = 0,
        .buffers = {},
        .images = {normalSamplerBinding},
    };

    renderer::DescriptorPool::updateDescriptorSets(descriptorPool_, {albedoSamplerUpdateInfo, normalSamplerUpdateInfo});
}

void engine::Engine::calculateDeltaTime() {
    thisFrameTime_ = std::chrono::high_resolution_clock::now();
    deltaTime_ = std::clamp(std::chrono::duration<float>(thisFrameTime_ - lastFrameTime_).count(), 0.0f, 0.1f);
    lastFrameTime_ = thisFrameTime_;
}

void engine::Engine::runPreTransferSystems() {
    using namespace ::components;

    ::systems::entities::updateControllers(*this);

    ::systems::integrateMovements(*this);
    ::systems::entities::sortEntities(*this);

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

    // renderer::CommandBuffer::bindVertexBuffers(commandBuffer, {worldTileMesh_.getMeshBuffer(), worldTileMesh_.getInstanceBuffer()}, {0, 0}, 0);
    // renderer::CommandBuffer::draw(commandBuffer, 4, static_cast<std::uint32_t>(worldTilePool_.data().size()), 0, 0);

    renderer::CommandBuffer::bindDescriptorSets(commandBuffer, renderer::DeviceOperation::GRAPHICS, pipelineLayout_, 0, {tilemapDescriptorSet_});
    renderer::CommandBuffer::bindVertexBuffers(commandBuffer, {entityTileMesh_.getMeshBuffer(), entityTileMesh_.getInstanceBuffer()}, {0, 0}, 0);
    renderer::CommandBuffer::draw(commandBuffer, 4, static_cast<std::uint32_t>(entityTilePool_.data().size()), 0, 0);

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
    renderer::PipelineLayout::destroy(pipelineLayout_);

    renderer::ImageView::destroy(tilemapAlbedoImageView_);
    renderer::ImageView::destroy(tilemapNormalImageView_);
    renderer::Image::destroy(tilemapAlbedoImage_);
    renderer::Image::destroy(tilemapNormalImage_);
    renderer::Sampler::destroy(sampler_);

    renderer::Buffer::destroy(cameraBuffer_);
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

    renderer::ShaderModuleCreateInfo tileVertShaderModuleCreateInfo = {
        .device = device,
        .data = tileVertShaderBinary,
    };

    renderer::ShaderModuleCreateInfo tileFragShaderModuleCreateInfo = {
        .device = device,
        .data = tileFragShaderBinary,
    };

    renderer::ShaderModule tileVertShaderModule = renderer::ShaderModule::create(tileVertShaderModuleCreateInfo);
    renderer::ShaderModule tileFragShaderModule = renderer::ShaderModule::create(tileFragShaderModuleCreateInfo);

    renderer::PipelineLayoutCreateInfo pipelineLayoutCreateInfo = {
        .device = device,
        .inputLayouts = {descriptorSetLayout_},
        .pushConstants = {},
    };

    pipelineLayout_ = renderer::PipelineLayout::create(pipelineLayoutCreateInfo);

    renderer::PipelineCreateInfo worldPipelineCreateInfo = {
        .renderPass = renderPass,
        .layout = pipelineLayout_,
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
                    .strideBytes = sizeof(TileInstance),
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
                    .format = renderer::VertexAttributeFormat::R32G32_FLOAT,
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
                    .format = renderer::VertexAttributeFormat::R32G32B32A32_FLOAT,
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
            .depthTestEnable = false,
            .depthWriteEnable = false,
            .depthBoundsTestEnable = false,
            .stencilTestEnable = false,
        },
        .multisample = {},
        .colourBlend = {
            .attachments = {renderer::ColourBlendAttachment{}},
        },
    };

    pipelines_ = renderer::Device::createPipelines(device, {worldPipelineCreateInfo});

    worldPipeline_ = pipelines_[0];

    renderer::ShaderModule::destroy(tileVertShaderModule);
    renderer::ShaderModule::destroy(tileFragShaderModule);
}