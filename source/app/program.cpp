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

        camera_ = data::makeUnique<game::Camera>(settings_, device_.ref(), transferCommandBuffer_.get(), transferQueue_.get(), swapchain_.ref());
        player_ = data::makeUnique<game::Player>(settings_, device_.ref(), transferCommandBuffer_.get(), transferQueue_.get());

        createBasicPipelineResources();

        lastFrameTime_ = std::chrono::high_resolution_clock::now();
    }

    void Program::update() {
        std::chrono::time_point<std::chrono::high_resolution_clock> currentTime = std::chrono::high_resolution_clock::now();

        std::chrono::duration<float> delta = currentTime - lastFrameTime_;

        float deltaTime = delta.count();

        lastFrameTime_ = currentTime;

        player_->update(deltaTime);

        camera_->slowMoveToPlayer(deltaTime, player_.ref());
        camera_->update();
    }

    void Program::render() {
        auto& commandBuffer = commandBuffers_[frameCounter_.index];
        auto& framebuffer = framebuffers_[imageCounter_.index];
        auto& inFlightFence = inFlightFences_[frameCounter_.index];
        auto& acquireSemaphore = acquireSemaphores_[frameCounter_.index];
        auto& presentSemaphore = presentSemaphores_[imageCounter_.index];

        commandBuffer.beginCapture();

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

        commandBuffer.beginRenderPass(renderPassBeginInfo);

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

        commandBuffer.bindPipeline(basicPipeline_.get());

        commandBuffer.setPipelineViewports({viewport}, 0);
        commandBuffer.setPipelineScissors({scissor}, 0);

        data::ReferenceList<renderer::DescriptorSet> sets = {
            player_->descriptorSet(),
            camera_->descriptorSet(),
        };

        commandBuffer.bindDescriptorSets(renderer::DeviceOperation::GRAPHICS, basicPipelineLayout_.ref(), 0, sets);

        player_->render(commandBuffer, basicPipelineLayout_.ref());

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
                player_->descriptorSetLayout(),
                camera_->descriptorSetLayout(),
            },
            .pushConstants = {
                player_->pushConstant(),
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
                        .strideBytes = sizeof(game::Vertex),
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
                .topology = renderer::PolygonTopology::TRIANGLE_STRIP,
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