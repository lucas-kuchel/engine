#include <app/program.hpp>

#include <fstream>

#include <stb_image.h>

namespace app {
    void Program::start() {
        renderer::CommandBufferCreateInfo transferCommandBuffersCreateInfo = {
            .count = 1,
        };

        transferCommandBuffers_ = transferCommandPool_->allocateCommandBuffers(transferCommandBuffersCreateInfo);
        transferCommandBuffer_ = transferCommandBuffers_.front();

        renderer::DescriptorSetInputInfo bufferInputInfo = {
            .type = renderer::DescriptorInputType::UNIFORM_BUFFER,
            .stageFlags = renderer::DescriptorShaderStageFlags::VERTEX,
            .count = 1,
            .binding = 0,
        };

        renderer::DescriptorSetLayoutCreateInfo layoutCreateInfo = {
            .device = device_.ref(),
            .inputs = {bufferInputInfo},
        };

        cameraDescriptorSetLayout_ = data::makeUnique<renderer::DescriptorSetLayout>(layoutCreateInfo);

        renderer::DescriptorPoolSize bufferSize = {
            .type = renderer::DescriptorInputType::UNIFORM_BUFFER,
            .count = 1,
        };

        renderer::DescriptorPoolCreateInfo poolCreateInfo = {
            .device = device_.ref(),
            .poolSizes = {bufferSize},
            .maximumSetCount = 1,
        };

        descriptorPool_ = data::makeUnique<renderer::DescriptorPool>(poolCreateInfo);

        renderer::DescriptorSetCreateInfo setCreateInfo = {
            .layouts = {cameraDescriptorSetLayout_.ref()},
        };

        descriptorSets_ = descriptorPool_->allocateDescriptorSets(setCreateInfo);
        basicDescriptorSet_ = descriptorSets_[0];

        renderer::BufferCreateInfo stagingBufferCreateInfo = {
            .device = device_.ref(),
            .memoryType = renderer::MemoryType::HOST_VISIBLE,
            .usageFlags = renderer::BufferUsageFlags::TRANSFER_SOURCE,
            .sizeBytes = 2048,
        };

        renderer::Buffer stagingBuffer(stagingBufferCreateInfo);

        std::uint64_t stagingBufferOffset = 0;

        transferCommandBuffer_->beginCapture();

        playerCharacter_.speed = settings_.controls.speed;

        playerController_.leftBinding = app::Key::A;
        playerController_.rightBinding = app::Key::D;

        playerCamera_.ease = settings_.camera.ease;
        playerCamera_.scale = settings_.camera.scale;

        game::createCharacter(playerCharacter_, device_.ref(), stagingBuffer, stagingBufferOffset, transferCommandBuffer_.get());
        game::createCamera(playerCamera_, device_.ref(), stagingBuffer, stagingBufferOffset, transferCommandBuffer_.get());

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
            .buffer = playerCamera_.buffer.ref(),
            .offsetBytes = 0,
            .rangeBytes = playerCamera_.buffer->size(),
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

        lastFrameTime_ = std::chrono::high_resolution_clock::now();
    }

    void Program::update() {
        std::chrono::time_point<std::chrono::high_resolution_clock> currentTime = std::chrono::high_resolution_clock::now();

        std::chrono::duration<float> delta = currentTime - lastFrameTime_;

        float deltaTime = delta.count();

        lastFrameTime_ = currentTime;

        game::updateCharacter(playerCharacter_, deltaTime);
        game::easeCameraTowardsCharacter(playerCamera_, playerCharacter_, deltaTime);

        renderer::BufferCreateInfo stagingBufferCreateInfo = {
            .device = device_.ref(),
            .memoryType = renderer::MemoryType::HOST_VISIBLE,
            .usageFlags = renderer::BufferUsageFlags::TRANSFER_SOURCE,
            .sizeBytes = 2048,
        };

        renderer::Buffer stagingBuffer(stagingBufferCreateInfo);

        std::uint64_t stagingBufferOffset = 0;

        transferCommandBuffer_->beginCapture();

        playerCamera_.extent = {swapchain_->extent().width, swapchain_->extent().height};

        game::updateCamera(playerCamera_, stagingBuffer, stagingBufferOffset, transferCommandBuffer_.get());

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
            .depthClearValue = {},
            .stencilClearValue = {},
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

        game::renderCharacter(playerCharacter_, commandBuffer, basicPipelineLayout_.ref());

        commandBuffer.endRenderPass();
        commandBuffer.endCapture();

        renderer::SubmitInfo submitInfo = {
            .fence = inFlightFence,
            .commandBuffers = {commandBuffer},
            .waits = {acquireSemaphore},
            .signals = {presentSemaphore},
            .waitFlags = {
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
                cameraDescriptorSetLayout_.ref(),
            },
            .pushConstants = {
                renderer::PushConstantInputInfo{
                    .sizeBytes = sizeof(glm::mat4),
                    .stageFlags = renderer::DescriptorShaderStageFlags::VERTEX,
                },
            },
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
                },
            },
            .inputAssembly = {
                .topology = renderer::PolygonTopology::TRIANGLE,
                .primitiveRestart = false,
            },
            .viewportCount = 1,
            .scissorCount = 1,
            .rasterisation = {},
            .multisample = {},
            .colourBlend = {
                .attachments = {renderer::ColourBlendAttachment{}},
            },
        };

        pipelines_ = device_->createPipelines({pipelineCreateInfo});

        basicPipeline_ = pipelines_.front();
    }
}