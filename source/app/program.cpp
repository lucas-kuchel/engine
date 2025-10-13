#include <app/program.hpp>

#include <cstring>
#include <fstream>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace app {
    void Program::start() {
        std::int32_t width = 0;
        std::int32_t height = 0;
        std::int32_t channels = 0;

        std::uint8_t* tilemapImageData = stbi_load("assets/images/tilemap.png", &width, &height, &channels, STBI_rgb_alpha);

        renderer::SamplerCreateInfo samplerCreateInfo = {
            .device = device_,
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
            .device = device_,
            .type = renderer::ImageType::IMAGE_2D,
            .format = renderer::ImageFormat::R8G8B8A8_UNORM,
            .memoryType = renderer::MemoryType::DEVICE_LOCAL,
            .usageFlags = renderer::ImageUsageFlags::SAMPLED | renderer::ImageUsageFlags::TRANSFER_DESTINATION,
            .extent = {static_cast<std::uint32_t>(width), static_cast<std::uint32_t>(height), 1},
            .sampleCount = 1,
            .mipLevels = 1,
            .arrayLayers = 1,
        };

        tilemapImage_ = renderer::Image::create(tilemapImageCreateInfo);

        transferCommandBuffers_ = renderer::CommandPool::allocateCommandBuffers(transferCommandPool_, 1);
        transferCommandBuffer_ = transferCommandBuffers_.front();

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
            .device = device_,
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
            .device = device_,
            .poolSizes = {bufferSize, imageSize},
            .maximumSetCount = 1,
        };

        descriptorPool_ = renderer::DescriptorPool::create(poolCreateInfo);

        renderer::DescriptorSetCreateInfo setCreateInfo = {
            .layouts = {descriptorSetLayout_},
        };

        descriptorSets_ = renderer::DescriptorPool::allocateDescriptorSets(descriptorPool_, setCreateInfo);
        basicDescriptorSet_ = descriptorSets_[0];

        controller_.leftBinding = app::Key::A;
        controller_.rightBinding = app::Key::D;
        controller_.jumpBinding = app::Key::SPACE;
        controller_.sprintBinding = app::Key::LSHIFT;

        camera_.ease = settings_.camera.ease;
        camera_.scale = settings_.camera.scale;

        characters_.push_back(game::Character{
            .baseSpeed = 3.0,
            .sprintMultiplier = 1.75f,
            .jumpForce = 8.0f,
        });

        characterInstances_.push_back(game::CharacterInstance{
            .position = {0.0f, 7.0f, 0.0f},
            .scale = {1.0f, 1.0f},
            .texOffset = {0.0f, 0.0f},
        });

        characterMovableBodies_.push_back(game::MovableBody{
            .position = {0.0f, 7.0f},
            .velocity = {0.0f, 0.0f},
            .acceleration = {0.0f, 0.0f},
        });

        characterColliders_.push_back(game::BoxCollider{
            .physics = {
                .friction = 0.1f,
            },
            .position = {0.0f, 7.0f},
            .extent = {1.0f, 1.0f},
        });

        characterCollisionResults_.emplace_back();

        characters_.push_back(game::Character{
            .baseSpeed = 4.5,
            .sprintMultiplier = 1.2f,
            .jumpForce = 9.5f,
        });

        characterInstances_.push_back(game::CharacterInstance{
            .position = {2.0f, 7.0f, 0.0f},
            .scale = {1.0f, 1.0f},
            .texOffset = {0.0f, 0.0f},
        });

        characterMovableBodies_.push_back(game::MovableBody{
            .position = {2.0f, 7.0f},
            .velocity = {0.0f, 0.0f},
            .acceleration = {0.0f, 0.0f},
        });

        characterColliders_.push_back(game::BoxCollider{
            .physics = {
                .friction = 0.05f,
            },
            .position = {2.0f, 7.0f},
            .extent = {1.0f, 1.0f},
        });

        characterCollisionResults_.emplace_back();

        game::loadMapFromFile(map_, "assets/maps/map0.json");

        std::uint64_t mapSizeBytes = map_.tiles.size() * sizeof(game::TileInstance);

        renderer::BufferCreateInfo stagingBufferCreateInfo = {
            .device = device_,
            .memoryType = renderer::MemoryType::HOST_VISIBLE,
            .usageFlags = renderer::BufferUsageFlags::TRANSFER_SOURCE,
            .sizeBytes = 8 * 1024 * 1024 + mapSizeBytes,
        };

        stagingBuffer_ = renderer::Buffer::create(stagingBufferCreateInfo);

        std::uint64_t stagingBufferOffset = 0;

        renderer::CommandBuffer::beginCapture(transferCommandBuffer_);

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

        renderer::CommandBuffer::pipelineBarrier(transferCommandBuffer_, renderer::PipelineStageFlags::TOP_OF_PIPE, renderer::PipelineStageFlags::TRANSFER, {memoryBarrier0});

        auto mapping = renderer::Buffer::map(stagingBuffer_, static_cast<std::uint32_t>(width * height * channels), stagingBufferOffset);

        std::memcpy(mapping.data.data(), tilemapImageData, static_cast<std::uint32_t>(width * height * channels));

        renderer::Buffer::unmap(stagingBuffer_, mapping);

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

        stagingBufferOffset += static_cast<std::uint32_t>(width * height * channels);

        renderer::CommandBuffer::copyBufferToImage(transferCommandBuffer_, stagingBuffer_, tilemapImage_, renderer::ImageLayout::TRANSFER_DESTINATION_OPTIMAL, {tilemapCopyRegion});

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

        renderer::CommandBuffer::pipelineBarrier(transferCommandBuffer_, renderer::PipelineStageFlags::TRANSFER, renderer::PipelineStageFlags::FRAGMENT_SHADER, {memoryBarrier1});

        game::createMap(tileMesh_, map_, device_, stagingBuffer_, stagingBufferOffset, transferCommandBuffer_);
        game::createCharacterInstances(characterMesh_, characterInstances_, device_, stagingBuffer_, stagingBufferOffset, transferCommandBuffer_);
        game::createCamera(camera_, device_, stagingBuffer_, stagingBufferOffset, transferCommandBuffer_);

        renderer::CommandBuffer::endCapture(transferCommandBuffer_);

        renderer::Fence fence = renderer::Fence::create({device_, 0});

        renderer::QueueSubmitInfo submitInfo = {
            .fence = fence,
            .commandBuffers = {transferCommandBuffer_},
            .waits = {},
            .signals = {},
            .waitFlags = {},
        };

        renderer::Queue::submit(transferQueue_, submitInfo);
        renderer::Device::waitForFences(device_, {fence});
        renderer::Fence::destroy(fence);

        stagingBufferFence_ = renderer::Fence::create({device_, renderer::FenceCreateFlags::START_SIGNALED});
        stagingBufferSemaphore_ = renderer::Semaphore::create(device_);

        createBasicPipelineResources();

        renderer::DescriptorSetBufferBinding cameraBufferBinding = {
            .buffer = camera_.uniformBuffer,
            .offsetBytes = 0,
            .rangeBytes = renderer::Buffer::size(camera_.uniformBuffer),
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

        stagingBufferCreateInfo.sizeBytes = 4 * 1024 * 1024;

        renderer::Buffer::destroy(stagingBuffer_);

        stagingBuffer_ = renderer::Buffer::create(stagingBufferCreateInfo);

        stbi_image_free(tilemapImageData);
    }

    void Program::update() {
        std::uint64_t stagingBufferOffset = 0;

        std::chrono::time_point<std::chrono::high_resolution_clock> currentTime = std::chrono::high_resolution_clock::now();

        std::chrono::duration<float> delta = currentTime - lastFrameTime_;

        float deltaTime = delta.count();

        lastFrameTime_ = currentTime;

        camera_.extent = renderer::Swapchain::getExtent(swapchain_);

        renderer::CommandBuffer::beginCapture(transferCommandBuffer_);

        auto& focusedCharacter = characters_[focusedCharacterIndex_];
        auto& focusedCharacterMovableBody = characterMovableBodies_[focusedCharacterIndex_];
        auto& focusedCharacterCollisionResult = characterCollisionResults_[focusedCharacterIndex_];
        auto& focusedCharacterInstance = characterInstances_[focusedCharacterIndex_];

        bool sprintKeyPressed = keysHeld_[keyIndex(controller_.sprintBinding)];
        bool leftKeyPressed = keysHeld_[keyIndex(controller_.leftBinding)];
        bool rightKeyPressed = keysHeld_[keyIndex(controller_.rightBinding)];
        bool jumpKeyPressed = keysHeld_[keyIndex(controller_.jumpBinding)];
        bool isColliding = focusedCharacterCollisionResult.collided;
        bool bottomCollision = focusedCharacterCollisionResult.bottom;

        if (sprintKeyPressed) {
            focusedCharacter.sprinting = true;
        }
        else {
            focusedCharacter.sprinting = false;
        }

        if (leftKeyPressed) {
            float multiplier = 1.0f;

            if (!focusedCharacterCollisionResult.bottom) {
                multiplier = 0.25f;
            }

            if (focusedCharacter.facing == game::CharacterFacing::RIGHT) {
                focusedCharacterInstance.scale.x *= -1.0f;
                focusedCharacter.facing = game::CharacterFacing::LEFT;
            }

            focusedCharacterMovableBody.acceleration.x -= multiplier * game::currentCharacterSpeed(focusedCharacter);
        }

        if (rightKeyPressed) {
            float multiplier = 1.0f;

            if (!focusedCharacterCollisionResult.bottom) {
                multiplier = 0.25f;
            }

            if (focusedCharacter.facing == game::CharacterFacing::LEFT) {
                focusedCharacterInstance.scale.x *= -1.0f;
                focusedCharacter.facing = game::CharacterFacing::RIGHT;
            }

            focusedCharacterMovableBody.acceleration.x += multiplier * game::currentCharacterSpeed(focusedCharacter);
        }

        if (jumpKeyPressed && isColliding && bottomCollision && focusedCharacterCollisionResult.bottom) {
            focusedCharacterMovableBody.velocity.y += focusedCharacter.jumpForce;
        }

        if (keysPressed_[keyIndex(Key::TAB)]) {
            if (focusedCharacterIndex_ + 1 == characters_.size()) {
                focusedCharacterIndex_ = 0;
            }
            else {
                focusedCharacterIndex_++;
            }
        }

        for (std::size_t i = 0; i < characters_.size(); i++) {
            float friction = 0.0f;

            const glm::vec2& acceleration = characterMovableBodies_[i].acceleration;
            const glm::vec2& velocity = characterMovableBodies_[i].velocity;

            bool accelerating = glm::length(acceleration) > 0.0f;
            bool reversing = glm::length(velocity) > 0.0f && glm::length(acceleration) > 0.0f && glm::dot(velocity, acceleration) < 0.0f;
            bool moving = glm::length(velocity) > 0.0f;

            if (characterCollisionResults_[i].collided && moving && (!accelerating || reversing)) {
                friction = (characterCollisionResults_[i].other->physics.friction + characterColliders_[i].physics.friction) * 0.5f;
            }

            game::updateMovement(characterMovableBodies_[i], deltaTime, map_.physics.gravity, friction, map_.physics.airResistance);

            characterInstances_[i].position = glm::vec3(characterMovableBodies_[i].position, 0.0f);
            characterColliders_[i].position = characterMovableBodies_[i].position;
        }

        game::resolveMapCollisions(map_, characterMovableBodies_, characterColliders_, characterCollisionResults_);
        game::updateCharacterInstances(characterMesh_, characterInstances_, stagingBuffer_, stagingBufferOffset, transferCommandBuffer_);
        game::easeCameraTowards(camera_, focusedCharacterMovableBody.position, deltaTime);
        game::updateCamera(camera_, stagingBuffer_, stagingBufferOffset, transferCommandBuffer_);

        renderer::CommandBuffer::endCapture(transferCommandBuffer_);

        renderer::QueueSubmitInfo submitInfo = {
            .fence = stagingBufferFence_,
            .commandBuffers = {transferCommandBuffer_},
            .waits = {},
            .signals = {stagingBufferSemaphore_},
            .waitFlags = {},
        };

        renderer::Queue::submit(transferQueue_, submitInfo);
    }

    void Program::render() {
        auto& commandBuffer = commandBuffers_[frameCounter_.index];
        auto& inFlightFence = inFlightFences_[frameCounter_.index];
        auto& acquireSemaphore = acquireSemaphores_[frameCounter_.index];

        auto& framebuffer = framebuffers_[imageCounter_.index];
        auto& presentSemaphore = presentSemaphores_[imageCounter_.index];

        renderer::CommandBuffer::reset(commandBuffer);

        renderer::Viewport viewport = {
            .position = {0.0, 0.0},
            .extent = renderer::Swapchain::getExtent(swapchain_),
            .minDepth = 0.0f,
            .maxDepth = 1.0f,
        };

        renderer::Scissor scissor = {
            .offset = {0, 0},
            .extent = renderer::Swapchain::getExtent(swapchain_),
        };

        renderer::RenderPassBeginInfo renderPassBeginInfo = {
            .renderPass = renderPass_,
            .framebuffer = framebuffer,
            .region = {
                .position = {0, 0},
                .extent = renderer::Swapchain::getExtent(swapchain_),
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

        game::renderMap(tileMesh_, map_, commandBuffer);
        game::renderCharacterInstances(characterMesh_, characterInstances_, commandBuffer);

        renderer::CommandBuffer::endRenderPass(commandBuffer);
        renderer::CommandBuffer::endCapture(commandBuffer);

        renderer::QueueSubmitInfo submitInfo = {
            .fence = inFlightFence,
            .commandBuffers = {commandBuffer},
            .waits = {stagingBufferSemaphore_, acquireSemaphore},
            .signals = {presentSemaphore},
            .waitFlags = {
                renderer::PipelineStageFlags::VERTEX_INPUT,
                renderer::PipelineStageFlags::COLOR_ATTACHMENT_OUTPUT,
            },
        };

        renderer::Queue::submit(graphicsQueue_, submitInfo);
    }

    void Program::close() {
        renderer::Device::waitIdle(device_);

        game::destroyCharacterInstances(characterMesh_);
        game::destroyMap(tileMesh_);
        game::destroyCamera(camera_);

        renderer::Fence::destroy(stagingBufferFence_);
        renderer::Semaphore::destroy(stagingBufferSemaphore_);

        renderer::DescriptorPool::destroy(descriptorPool_);
        renderer::DescriptorSetLayout::destroy(descriptorSetLayout_);
        renderer::Pipeline::destroy(basicPipeline_);
        renderer::PipelineLayout::destroy(basicPipelineLayout_);
        renderer::Buffer::destroy(stagingBuffer_);
        renderer::ImageView::destroy(tilemapImageView_);
        renderer::Image::destroy(tilemapImage_);
        renderer::Sampler::destroy(sampler_);
    }

    void Program::createBasicPipelineResources() {
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
            .device = device_,
            .data = vertexShaderBinary,
        };

        renderer::ShaderModuleCreateInfo fragmentShaderModuleCreateInfo = {
            .device = device_,
            .data = fragmentShaderBinary,
        };

        renderer::ShaderModule vertexShaderModule = renderer::ShaderModule::create(vertexShaderModuleCreateInfo);
        renderer::ShaderModule fragmentShaderModule = renderer::ShaderModule::create(fragmentShaderModuleCreateInfo);

        renderer::PipelineLayoutCreateInfo pipelineLayoutCreateInfo = {
            .device = device_,
            .inputLayouts = {descriptorSetLayout_},
            .pushConstants = {},
        };

        basicPipelineLayout_ = renderer::PipelineLayout::create(pipelineLayoutCreateInfo);

        renderer::PipelineCreateInfo pipelineCreateInfo = {
            .renderPass = renderPass_,
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
                        .strideBytes = sizeof(game::CharacterVertex),
                    },
                    renderer::VertexInputBindingDescription{
                        .inputRate = renderer::VertexInputRate::PER_INSTANCE,
                        .binding = 1,
                        .strideBytes = sizeof(game::CharacterInstance),
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
                        .binding = 0,
                        .location = 1,
                    },
                    renderer::VertexAttributeDescription{
                        .format = renderer::VertexAttributeFormat::R32G32B32_FLOAT,
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
                        .format = renderer::VertexAttributeFormat::R32_FLOAT,
                        .binding = 1,
                        .location = 5,
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

        pipelines_ = renderer::Device::createPipelines(device_, {pipelineCreateInfo});

        basicPipeline_ = pipelines_.front();

        renderer::ShaderModule::destroy(vertexShaderModule);
        renderer::ShaderModule::destroy(fragmentShaderModule);
    }
}