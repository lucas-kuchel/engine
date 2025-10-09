#include <app/program.hpp>

#include <cstring>
#include <fstream>

#include <stb_image.h>

namespace app {
    void Program::start() {
        renderer::CommandBufferCreateInfo transferCommandBuffersCreateInfo = {
            .count = 1,
        };

        std::int32_t width = 0;
        std::int32_t height = 0;
        std::int32_t channels = 0;

        std::uint8_t* tilemapImageData = stbi_load("assets/images/tilemap.png", &width, &height, &channels, STBI_rgb_alpha);

        renderer::SamplerCreateInfo samplerCreateInfo = {
            .device = device_.ref(),
            .minFilter = renderer::Filter::NEAREST,
            .magFilter = renderer::Filter::NEAREST,
            .mipmapMode = renderer::MipmapMode::NEAREST,
            .addressModeU = renderer::AddressMode::REPEAT,
            .addressModeV = renderer::AddressMode::REPEAT,
            .addressModeW = renderer::AddressMode::REPEAT,
            .borderColour = renderer::BorderColour::FLOAT_OPAQUE_BLACK,
            .enableAnisotropy = false,
            .maxAnisotropy = 0.0f,
            .enableCompare = false,
            .unnormalisedCoordinates = false,
            .comparison = renderer::CompareOperation::ALWAYS,
            .mipLodBias = 0.0f,
            .minLod = 0.0f,
            .maxLod = 0.0f,
        };

        sampler_ = data::makeUnique<renderer::Sampler>(samplerCreateInfo);

        renderer::ImageCreateInfo tilemapImageCreateInfo = {
            .device = device_.ref(),
            .type = renderer::ImageType::IMAGE_2D,
            .format = renderer::ImageFormat::R8G8B8A8_UNORM,
            .memoryType = renderer::MemoryType::DEVICE_LOCAL,
            .usageFlags = renderer::ImageUsageFlags::SAMPLED | renderer::ImageUsageFlags::TRANSFER_DESTINATION,
            .extent = {static_cast<std::uint32_t>(width), static_cast<std::uint32_t>(height), 1},
            .sampleCount = 1,
            .mipLevels = 1,
            .arrayLayers = 1,
        };

        tilemapImage_ = data::makeUnique<renderer::Image>(tilemapImageCreateInfo);

        transferCommandBuffers_ = transferCommandPool_->allocateCommandBuffers(transferCommandBuffersCreateInfo);
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
            .device = device_.ref(),
            .inputs = {bufferInputInfo, samplerInputInfo},
        };

        descriptorSetLayout_ = data::makeUnique<renderer::DescriptorSetLayout>(layoutCreateInfo);

        renderer::DescriptorPoolSize bufferSize = {
            .type = renderer::DescriptorInputType::UNIFORM_BUFFER,
            .count = 1,
        };

        renderer::DescriptorPoolSize imageSize = {
            .type = renderer::DescriptorInputType::IMAGE_SAMPLER,
            .count = 1,
        };

        renderer::DescriptorPoolCreateInfo poolCreateInfo = {
            .device = device_.ref(),
            .poolSizes = {bufferSize, imageSize},
            .maximumSetCount = 1,
        };

        descriptorPool_ = data::makeUnique<renderer::DescriptorPool>(poolCreateInfo);

        renderer::DescriptorSetCreateInfo setCreateInfo = {
            .layouts = {descriptorSetLayout_.ref()},
        };

        descriptorSets_ = descriptorPool_->allocateDescriptorSets(setCreateInfo);
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
            .texScale = 1.0f,
        });

        characterMovableBodies_.push_back(game::MovableBody{
            .position = {0.0f, 7.0f},
            .velocity = {0.0f, 0.0f},
            .acceleration = {0.0f, 0.0f},
        });

        characterColliders_.push_back(game::BoxCollider{
            .physics = {
                .friction = 1.00f,
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
            .texScale = 1.0f,
        });

        characterMovableBodies_.push_back(game::MovableBody{
            .position = {2.0f, 7.0f},
            .velocity = {0.0f, 0.0f},
            .acceleration = {0.0f, 0.0f},
        });

        characterColliders_.push_back(game::BoxCollider{
            .physics = {
                .friction = 1.00f,
            },
            .position = {2.0f, 7.0f},
            .extent = {1.0f, 1.0f},
        });

        characterCollisionResults_.emplace_back();

        game::loadMapFromFile(map_, "assets/maps/map0.json");

        std::uint64_t mapSizeBytes = map_.tiles.size() * sizeof(game::TileInstance);

        renderer::BufferCreateInfo stagingBufferCreateInfo = {
            .device = device_.ref(),
            .memoryType = renderer::MemoryType::HOST_VISIBLE,
            .usageFlags = renderer::BufferUsageFlags::TRANSFER_SOURCE,
            .sizeBytes = 8 * 1024 * 1024 + mapSizeBytes,
        };

        stagingBuffer_ = data::makeUnique<renderer::Buffer>(stagingBufferCreateInfo);

        std::uint64_t stagingBufferOffset = 0;

        transferCommandBuffer_->beginCapture();

        renderer::ImageMemoryBarrier memoryBarrier0 = {
            .image = tilemapImage_.ref(),
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

        transferCommandBuffer_->pipelineBarrier(renderer::PipelineStageFlags::TOP_OF_PIPE, renderer::PipelineStageFlags::TRANSFER, {memoryBarrier0});

        auto tilemapStagingMapping = stagingBuffer_->map(static_cast<std::uint32_t>(width * height * channels), stagingBufferOffset);

        std::memcpy(tilemapStagingMapping.data(), tilemapImageData, static_cast<std::uint32_t>(width * height * channels));

        stagingBuffer_->unmap();

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

        transferCommandBuffer_->copyBufferToImage(stagingBuffer_.ref(), tilemapImage_.ref(), renderer::ImageLayout::TRANSFER_DESTINATION_OPTIMAL, {tilemapCopyRegion});

        renderer::ImageMemoryBarrier memoryBarrier1 = {
            .image = tilemapImage_.ref(),
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

        transferCommandBuffer_->pipelineBarrier(renderer::PipelineStageFlags::TRANSFER, renderer::PipelineStageFlags::FRAGMENT_SHADER, {memoryBarrier1});

        game::createMap(tileMesh_, map_, device_.ref(), stagingBuffer_.ref(), stagingBufferOffset, transferCommandBuffer_.get());
        game::createCharacterInstances(characterMesh_, characterInstances_, device_.ref(), stagingBuffer_.ref(), stagingBufferOffset, transferCommandBuffer_.get());
        game::createCamera(camera_, device_.ref(), stagingBuffer_.ref(), stagingBufferOffset, transferCommandBuffer_.get());

        transferCommandBuffer_->endCapture();

        renderer::Fence fence({device_.ref(), 0});

        renderer::SubmitInfo submitInfo = {
            .fence = fence,
            .commandBuffers = {transferCommandBuffer_.get()},
            .waits = {},
            .signals = {},
            .waitFlags = {},
        };

        transferQueue_->submit(submitInfo);

        device_->waitForFences({fence});

        createBasicPipelineResources();

        renderer::DescriptorSetBufferBinding cameraBufferBinding = {
            .buffer = camera_.uniformBuffer.ref(),
            .offsetBytes = 0,
            .rangeBytes = camera_.uniformBuffer->size(),
        };

        renderer::DescriptorSetUpdateInfo uniformBufferUpdateInfo = {
            .set = basicDescriptorSet_.get(),
            .inputType = renderer::DescriptorInputType::UNIFORM_BUFFER,
            .binding = 0,
            .arrayElement = 0,
            .buffers = {cameraBufferBinding},
            .images = {},
        };

        descriptorPool_->updateDescriptorSets({uniformBufferUpdateInfo});

        renderer::ImageViewCreateInfo tilemapImageViewCreateInfo = {
            .image = tilemapImage_.ref(),
            .type = renderer::ImageViewType::IMAGE_2D,
            .aspectFlags = renderer::ImageAspectFlags::COLOUR,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1,
        };

        tilemapImageView_ = data::makeUnique<renderer::ImageView>(tilemapImageViewCreateInfo);

        renderer::DescriptorSetImageBinding samplerBinding = {
            .image = tilemapImageView_.ref(),
            .sampler = sampler_.ref(),
            .layout = renderer::ImageLayout::SHADER_READ_ONLY_OPTIMAL,
        };

        renderer::DescriptorSetUpdateInfo samplerUpdateInfo = {
            .set = basicDescriptorSet_.get(),
            .inputType = renderer::DescriptorInputType::IMAGE_SAMPLER,
            .binding = 1,
            .arrayElement = 0,
            .buffers = {},
            .images = {samplerBinding},
        };

        descriptorPool_->updateDescriptorSets({samplerUpdateInfo});

        lastFrameTime_ = std::chrono::high_resolution_clock::now();

        stagingBufferCreateInfo.sizeBytes = 4 * 1024 * 1024;

        stagingBuffer_.reset();
        stagingBuffer_ = data::makeUnique<renderer::Buffer>(stagingBufferCreateInfo);

        stbi_image_free(tilemapImageData);
    }

    void Program::update() {
        std::uint64_t stagingBufferOffset = 0;

        std::chrono::time_point<std::chrono::high_resolution_clock> currentTime = std::chrono::high_resolution_clock::now();

        std::chrono::duration<float> delta = currentTime - lastFrameTime_;

        float deltaTime = delta.count();

        lastFrameTime_ = currentTime;

        camera_.extent = {swapchain_->extent().width, swapchain_->extent().height};

        transferCommandBuffer_->beginCapture();

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
                friction = (characterCollisionResults_[i].other.get().physics.friction + characterColliders_[i].physics.friction) * 0.5f;
            }

            game::updateMovement(characterMovableBodies_[i], deltaTime, map_.physics.gravity, friction, map_.physics.airResistance);

            characterInstances_[i].position = glm::vec3(characterMovableBodies_[i].position, 0.0f);
            characterColliders_[i].position = characterMovableBodies_[i].position;
        }

        game::resolveMapCollisions(map_, characterMovableBodies_, characterColliders_, characterCollisionResults_);

        game::updateCharacterInstances(characterMesh_, characterInstances_, stagingBuffer_.ref(), stagingBufferOffset, transferCommandBuffer_.get());

        game::easeCameraTowards(camera_, focusedCharacterMovableBody.position, deltaTime);
        game::updateCamera(camera_, stagingBuffer_.ref(), stagingBufferOffset, transferCommandBuffer_.get());

        transferCommandBuffer_->endCapture();

        renderer::SubmitInfo submitInfo = {
            .fence = stagingBufferFence_.ref(),
            .commandBuffers = {transferCommandBuffer_.get()},
            .waits = {},
            .signals = {stagingBufferSemaphore_.ref()},
            .waitFlags = {},
        };

        transferQueue_->submit(submitInfo);
    }

    void Program::render() {
        auto& commandBuffer = commandBuffers_[frameCounter_.index];
        auto& framebuffer = framebuffers_[imageCounter_.index];
        auto& inFlightFence = inFlightFences_[frameCounter_.index];
        auto& acquireSemaphore = acquireSemaphores_[frameCounter_.index];
        auto& presentSemaphore = presentSemaphores_[imageCounter_.index];

        data::Rect2D<std::int32_t, std::uint32_t> renderArea = {
            .offset = {0, 0},
            .extent = swapchain_->extent(),
        };

        data::ColourRGBA clearColour = {
            .r = 0.0f,
            .g = 0.2f,
            .b = 0.9f,
            .a = 1.0f,
        };

        renderer::RenderPassBeginInfo renderPassBeginInfo = {
            .renderPass = renderPass_.ref(),
            .framebuffer = framebuffer,
            .renderArea = renderArea,
            .clearValues = {
                clearColour,
            },
            .depthClearValue = 0.0f,
            .stencilClearValue = 0u,
        };

        renderer::Viewport viewport = {
            .position = {
                .x = 0.0,
                .y = 0.0,
            },
            .extent = {
                .width = static_cast<float>(swapchain_->extent().width),
                .height = static_cast<float>(swapchain_->extent().height),
            },
            .depth = {
                .min = 0.0,
                .max = 1.0,
            },
        };

        renderer::Scissor scissor = {
            .offset = {0, 0},
            .extent = swapchain_->extent(),
        };

        commandBuffer.beginCapture();
        commandBuffer.beginRenderPass(renderPassBeginInfo);
        commandBuffer.bindPipeline(basicPipeline_.get());

        commandBuffer.setPipelineViewports({viewport}, 0);
        commandBuffer.setPipelineScissors({scissor}, 0);

        commandBuffer.bindDescriptorSets(renderer::DeviceOperation::GRAPHICS, basicPipelineLayout_.ref(), 0, {basicDescriptorSet_.get()});

        game::renderMap(tileMesh_, map_, commandBuffer);

        game::renderCharacterInstances(characterMesh_, characterInstances_, commandBuffer);

        commandBuffer.endRenderPass();
        commandBuffer.endCapture();

        renderer::SubmitInfo submitInfo = {
            .fence = inFlightFence,
            .commandBuffers = {commandBuffer},
            .waits = {stagingBufferSemaphore_.ref(), acquireSemaphore},
            .signals = {presentSemaphore},
            .waitFlags = {
                renderer::PipelineStageFlags::VERTEX_INPUT,
                renderer::PipelineStageFlags::COLOR_ATTACHMENT_OUTPUT,
            },
        };

        graphicsQueue_->submit(submitInfo);
    }

    void Program::close() {
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
            .device = device_.ref(),
            .data = vertexShaderBinary,
        };

        renderer::ShaderModuleCreateInfo fragmentShaderModuleCreateInfo = {
            .device = device_.ref(),
            .data = fragmentShaderBinary,
        };

        renderer::ShaderModule vertexShaderModule(vertexShaderModuleCreateInfo);
        renderer::ShaderModule fragmentShaderModule(fragmentShaderModuleCreateInfo);

        renderer::PipelineLayoutCreateInfo pipelineLayoutCreateInfo = {
            .device = device_.ref(),
            .inputLayouts = {
                descriptorSetLayout_.ref(),
            },
            .pushConstants = {},
        };

        basicPipelineLayout_ = data::makeUnique<renderer::PipelineLayout>(pipelineLayoutCreateInfo);

        renderer::PipelineCreateInfo pipelineCreateInfo = {
            .renderPass = renderPass_.ref(),
            .layout = basicPipelineLayout_.ref(),
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

        pipelines_ = device_->createPipelines({pipelineCreateInfo});

        basicPipeline_ = pipelines_.front();
    }
}