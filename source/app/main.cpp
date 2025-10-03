#include <app/context.hpp>
#include <app/window.hpp>

#include <renderer/device.hpp>
#include <renderer/instance.hpp>
#include <renderer/queue.hpp>
#include <renderer/surface.hpp>
#include <renderer/swapchain.hpp>

#include <renderer/commands/buffer.hpp>
#include <renderer/commands/pool.hpp>

#include <renderer/resources/buffer.hpp>
#include <renderer/resources/fence.hpp>
#include <renderer/resources/framebuffer.hpp>
#include <renderer/resources/image.hpp>
#include <renderer/resources/pass.hpp>
#include <renderer/resources/pipeline.hpp>
#include <renderer/resources/semaphore.hpp>
#include <renderer/resources/shader.hpp>

#include <filesystem/file.hpp>

#include <chrono>
#include <cmath>
#include <cstring>
#include <exception>
#include <print>

void run() {
    std::uint32_t imageCount = 3;
    std::uint32_t frameCount = 3;
    std::uint32_t frameIndex = 0;

    bool synchronise = false;

    app::Context context;

    app::WindowCreateInfo windowCreateInfo = {
        .context = context,
        .extent = {1280, 720},
        .title = "Vulkan Test - Engine",
        .visibility = app::WindowVisibility::WINDOWED,
        .resizable = true,
    };

    app::Window window(windowCreateInfo);

    renderer::InstanceCreateInfo instanceCreateInfo = {
        .applicationName = window.getTitle(),
        .applicationVersion = data::Version(0, 0, 1),
        .engineName = "Engine",
        .engineVersion = data::Version(0, 0, 1),
        .requestDebug = true,
    };

    renderer::Instance instance(instanceCreateInfo);

    renderer::SurfaceCreateInfo surfaceCreateInfo = {
        .instance = instance,
        .window = window,
    };

    renderer::Surface surface(surfaceCreateInfo);

    renderer::DeviceCreateInfo deviceCreateInfo = {
        .instance = instance,
        .queues = {
            renderer::QueueCreateInfo{
                .flags = renderer::QueueFlags::RENDER | renderer::QueueFlags::TRANSFER,
                .surface = nullptr,
            },
            renderer::QueueCreateInfo{
                .flags = renderer::QueueFlags::PRESENT,
                .surface = surface,
            },
        },
    };

    renderer::Device device(deviceCreateInfo);

    std::span<renderer::Queue> queues = device.getQueues();

    renderer::Queue& renderQueue = queues[0];
    renderer::Queue& presentQueue = queues[1];

    renderer::SwapchainCreateInfo swapchainCreateInfo = {
        .instance = instance,
        .surface = surface,
        .device = device,
        .presentQueue = presentQueue,
        .renderQueue = renderQueue,
        .imageCount = imageCount,
        .synchronise = synchronise,
    };

    renderer::Swapchain swapchain(swapchainCreateInfo);

    imageCount = swapchain.getImageCount();
    frameCount = std::min(imageCount, frameCount);

    renderer::CommandPoolCreateInfo commandPoolCreateInfo = {
        .device = device,
        .queue = renderQueue,
    };

    renderer::CommandPool commandPool(commandPoolCreateInfo);

    renderer::CommandBufferCreateInfo commandBufferCreateInfo = {
        .level = renderer::CommandBufferLevel::PRIMARY,
        .count = frameCount,
    };

    std::vector<renderer::CommandBuffer> commandBuffers = commandPool.allocateCommandBuffers(commandBufferCreateInfo);

    renderer::RenderPassCreateInfo renderPassCreateInfo = {
        .device = device,
        .colourAttachments = {
            renderer::FrameAttachmentInfo{
                .format = swapchain.getImageFormat(),
                .loadOperation = renderer::LoadOperation::CLEAR,
                .storeOperation = renderer::StoreOperation::STORE,
                .sampleCount = 1,
            },
        },
        .depthAttachment = {},
        .stencilAttachment = {},
    };

    renderer::RenderPass renderPass(renderPassCreateInfo);

    std::vector<renderer::Semaphore> acquireSemaphores;
    std::vector<renderer::Semaphore> submitSemaphores;
    std::vector<renderer::Fence> fences;

    std::vector<renderer::Framebuffer> framebuffers;
    std::vector<data::NullableReference<renderer::Fence>> perImageFences;

    acquireSemaphores.reserve(frameCount);
    fences.reserve(frameCount);

    submitSemaphores.reserve(imageCount);
    framebuffers.reserve(imageCount);
    perImageFences.resize(imageCount);

    auto swapchainImageViews = swapchain.getImageViews();

    renderer::SemaphoreCreateInfo semaphoreCreateInfo = {
        .device = device,
    };

    renderer::FenceCreateInfo fenceCreateInfo = {
        .device = device,
        .startSignaled = true,
    };

    for (std::uint32_t i = 0; i < imageCount; i++) {
        renderer::FramebufferCreateInfo framebufferCreateInfo = {
            .device = device,
            .renderPass = renderPass,
            .imageViews = {swapchainImageViews[i]},
        };

        framebuffers.emplace_back(framebufferCreateInfo);
        submitSemaphores.emplace_back(semaphoreCreateInfo);
    }

    for (std::uint32_t i = 0; i < frameCount; i++) {
        acquireSemaphores.emplace_back(semaphoreCreateInfo);
        fences.emplace_back(fenceCreateInfo);
    }

    filesystem::BinaryFile<std::uint32_t> vertexShaderFile("assets/shaders/basic.vert.spv");
    filesystem::BinaryFile<std::uint32_t> fragmentShaderFile("assets/shaders/basic.frag.spv");

    std::vector vertexShaderBinary = vertexShaderFile.read();
    std::vector fragmentShaderBinary = fragmentShaderFile.read();

    renderer::ShaderModuleCreateInfo vertexShaderModuleCreateInfo = {
        .device = device,
        .data = vertexShaderBinary,
    };

    renderer::ShaderModuleCreateInfo fragmentShaderModuleCreateInfo = {
        .device = device,
        .data = fragmentShaderBinary,
    };

    renderer::ShaderModule vertexShaderModule(vertexShaderModuleCreateInfo);
    renderer::ShaderModule fragmentShaderModule(fragmentShaderModuleCreateInfo);

    struct Vertex {
        data::Position2D<float> position;
    };

    struct Triangle {
        std::uint32_t a, b, c;
    };

    std::array<Vertex, 4> vertices = {
        Vertex({0.0, -0.5}),
        Vertex({-0.5, 0.5}),
        Vertex({0.5, 0.5}),
    };

    std::array<Triangle, 2> triangles = {
        Triangle{0, 1, 2},
    };

    renderer::BufferCreateInfo vertexBufferCreateInfo = {
        .device = device,
        .type = renderer::MemoryType::DEVICE_LOCAL,
        .sizeBytes = vertices.size() * sizeof(Vertex),
        .usage = renderer::BufferUsageFlags::VERTEX | renderer::BufferUsageFlags::TRANSFER_DESTINATION,
    };

    renderer::BufferCreateInfo indexBufferCreateInfo = {
        .device = device,
        .type = renderer::MemoryType::DEVICE_LOCAL,
        .sizeBytes = triangles.size() * sizeof(Triangle),
        .usage = renderer::BufferUsageFlags::INDEX | renderer::BufferUsageFlags::TRANSFER_DESTINATION,
    };

    renderer::Buffer vertexBuffer(vertexBufferCreateInfo);
    renderer::Buffer indexBuffer(indexBufferCreateInfo);

    {
        renderer::CommandBufferCreateInfo transferCommandBuffersCreateInfo = {
            .level = renderer::CommandBufferLevel::PRIMARY,
            .count = 1,
        };

        std::vector<renderer::CommandBuffer> transferCommandBuffers = commandPool.allocateCommandBuffers(transferCommandBuffersCreateInfo);

        auto& transferCommandBuffer = transferCommandBuffers.front();

        renderer::BufferCreateInfo vertexStagingBufferCreateInfo = {
            .device = device,
            .type = renderer::MemoryType::HOST_VISIBLE,
            .sizeBytes = vertices.size() * sizeof(Vertex),
            .usage = renderer::BufferUsageFlags::TRANSFER_SOURCE,
        };

        renderer::BufferCreateInfo indexStagingBufferCreateInfo = {
            .device = device,
            .type = renderer::MemoryType::HOST_VISIBLE,
            .sizeBytes = triangles.size() * sizeof(Triangle),
            .usage = renderer::BufferUsageFlags::TRANSFER_SOURCE,
        };

        renderer::Buffer vertexStagingBuffer(vertexStagingBufferCreateInfo);
        renderer::Buffer indexStagingBuffer(indexStagingBufferCreateInfo);

        auto vertexStagingBufferMapping = vertexStagingBuffer.map(vertexStagingBuffer.getSize(), 0);
        auto indexStagingBufferMapping = indexStagingBuffer.map(indexStagingBuffer.getSize(), 0);

        auto vertexMemory = vertexStagingBufferMapping.get();
        auto indexMemory = indexStagingBufferMapping.get();

        std::memcpy(vertexMemory.data(), vertices.data(), vertexStagingBuffer.getSize());
        std::memcpy(indexMemory.data(), triangles.data(), indexStagingBuffer.getSize());

        vertexStagingBufferMapping.unmap();
        indexStagingBufferMapping.unmap();

        auto transferCommandBufferCapture = transferCommandBuffer.beginCapture();

        renderer::BufferCopyRegion vertexCopyRegion = {
            .sourceOffset = 0,
            .destinationOffset = 0,
            .size = vertexStagingBuffer.getSize()};

        renderer::BufferCopyRegion indexCopyRegion = {
            .sourceOffset = 0,
            .destinationOffset = 0,
            .size = indexStagingBuffer.getSize()};

        transferCommandBufferCapture.copyBuffer(vertexStagingBuffer, vertexBuffer, {vertexCopyRegion});
        transferCommandBufferCapture.copyBuffer(indexStagingBuffer, indexBuffer, {indexCopyRegion});

        transferCommandBufferCapture.end();

        renderer::Fence fence({device, false});

        renderer::SubmitInfo submitInfo = {
            .fence = fence,
            .commandBuffers = {transferCommandBuffer},
            .waits = {},
            .signals = {},
            .waitFlags = {},
        };

        renderQueue.submit(submitInfo);

        device.waitForFences({fence});
    }

    data::ColourRGB materialColour = {
        .r = 0.64f,
        .g = 0.21f,
        .b = 0.85f,
    };

    renderer::BufferCreateInfo materialBufferCreateInfo = {
        .device = device,
        .type = renderer::MemoryType::HOST_VISIBLE,
        .sizeBytes = sizeof(data::ColourRGB),
        .usage = renderer::BufferUsageFlags::UNIFORM,
    };

    renderer::Buffer materialBuffer(materialBufferCreateInfo);

    {
        auto materialBufferMapping = materialBuffer.map(materialBuffer.getSize(), 0);
        auto materialMemory = materialBufferMapping.get();

        std::memcpy(materialMemory.data(), &materialColour, materialBuffer.getSize());

        materialBufferMapping.unmap();
    }

    renderer::DescriptorSetInputInfo inputInfo = {
        .type = renderer::DescriptorInputType::UNIFORM_BUFFER,
        .count = 1,
        .binding = 0,
        .stageFlags = renderer::DescriptorShaderStageFlags::FRAGMENT,
    };

    renderer::DescriptorSetLayoutCreateInfo layoutCreateInfo = {
        .device = device,
        .inputs = {inputInfo},
    };

    renderer::DescriptorSetLayout layout(layoutCreateInfo);

    renderer::DescriptorPoolSize size = {
        .type = renderer::DescriptorInputType::UNIFORM_BUFFER,
        .count = 1,
    };

    renderer::DescriptorPoolCreateInfo poolCreateInfo = {
        .device = device,
        .poolSizes = {size},
        .maximumSetCount = 1,
    };

    renderer::DescriptorPool descriptorPool(poolCreateInfo);

    renderer::DescriptorSetCreateInfo setCreateInfo = {
        .layouts = {layout},
    };

    auto descriptorSets = descriptorPool.allocateDescriptorSets(setCreateInfo);

    auto& descriptorSet = descriptorSets.front();

    renderer::DescriptorSetBufferBinding bufferBinding = {
        .buffer = materialBuffer,
        .offset = 0,
        .range = materialBuffer.getSize(),
    };

    renderer::DescriptorSetUpdateInfo setUpdateInfo = {
        .set = descriptorSet,
        .binding = 0,
        .arrayElement = 0,
        .inputType = renderer::DescriptorInputType::UNIFORM_BUFFER,
        .buffers = {bufferBinding},
    };

    descriptorPool.updateDescriptorSets({setUpdateInfo});

    renderer::PipelineLayoutCreateInfo pipelineLayoutCreateInfo = {
        .device = device,
        .inputLayouts = {
            layout,
        },
        .pushConstants = {
            renderer::PushConstantInputInfo{
                .sizeBytes = 8,
                .stageFlags = renderer::DescriptorShaderStageFlags::VERTEX,
            },
        },
    };

    renderer::PipelineLayout pipelineLayout(pipelineLayoutCreateInfo);

    renderer::PipelineCreateInfo pipelineCreateInfo = {
        .renderPass = renderPass,
        .layout = pipelineLayout,
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
                    .binding = 0,
                    .strideBytes = sizeof(Vertex),
                    .inputRate = renderer::VertexInputRate::PER_VERTEX,
                },
            },
            .attributes = {
                renderer::VertexAttributeDescription{
                    .format = renderer::VertexAttributeFormat::R32G32_FLOAT,
                    .location = 0,
                    .binding = 0,
                },
                renderer::VertexAttributeDescription{
                    .format = renderer::VertexAttributeFormat::R32G32B32_FLOAT,
                    .location = 1,
                    .binding = 0,
                },
            },
        },
        .inputAssembly = {
            .topology = renderer::RasterisationPrimitive::TRIANGLE_STRIP,
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

    std::vector<renderer::Pipeline> pipelines = device.createPipelines({pipelineCreateInfo});

    renderer::Pipeline& pipeline = pipelines.front();

    bool running = true;

    data::ColourRGBA attachmentClearColour = {
        .r = 0.19f,
        .g = 0.37f,
        .b = 0.74f,
        .a = 1.0f,
    };

    data::Position2D<float> pushConstant = {
        .x = 0.0f,
        .y = 0.0f,
    };

    float angle = 0.0f;
    const float radius = 0.5f;
    const float speed = 1.0f;

    auto lastTime = std::chrono::high_resolution_clock::now();

    while (running) {
        auto currentTime = std::chrono::high_resolution_clock::now();
        float deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();

        lastTime = currentTime;

        angle += speed * deltaTime;
        if (angle > 2.0f * M_PIf) {
            angle -= 2.0f * M_PIf;
        }

        pushConstant.x = std::cos(angle) * radius;
        pushConstant.y = std::sin(angle) * radius;

        context.pollEvents();

        while (window.hasEvents()) {
            app::WindowEvent event = window.getNextEvent();

            switch (event.type) {
                case app::WindowEventType::CLOSED:
                    running = false;
                    break;

                default:
                    break;
            }
        }

        auto& fence = fences[frameIndex];
        auto& acquireSemaphore = acquireSemaphores[frameIndex];
        auto& commandBuffer = commandBuffers[frameIndex];

        device.waitForFences({fence});
        device.resetFences({fence});

        std::uint32_t imageIndex = 0;

        bool resized = false;

        while (true) {
            if (swapchain.shouldRecreate()) {
                device.waitIdle();

                renderer::SwapchainRecreateInfo swapchainRecreateInfo = {
                    .imageCount = imageCount,
                    .synchronise = synchronise,
                };

                swapchain.recreate(swapchainRecreateInfo);

                resized = true;
            }

            imageIndex = swapchain.acquireNextImage(acquireSemaphore);

            if (!swapchain.shouldRecreate()) {
                break;
            }
        }

        if (resized) {
            swapchainImageViews = swapchain.getImageViews();

            framebuffers.clear();
            submitSemaphores.clear();

            imageCount = swapchain.getImageCount();

            perImageFences.resize(imageCount);

            for (std::uint32_t i = 0; i < imageCount; i++) {
                renderer::FramebufferCreateInfo framebufferCreateInfo = {
                    .device = device,
                    .renderPass = renderPass,
                    .imageViews = {swapchainImageViews[i]},
                };

                framebuffers.emplace_back(framebufferCreateInfo);
                submitSemaphores.emplace_back(semaphoreCreateInfo);
            }
        }

        auto& framebuffer = framebuffers[imageIndex];
        auto& submitSemaphore = submitSemaphores[imageIndex];

        renderer::RenderPassBeginInfo renderPassBeginInfo = {
            .renderPass = renderPass,
            .framebuffer = framebuffer,
            .renderArea = {
                .offset = {0, 0},
                .extent = swapchain.getExtent(),
            },
            .clearValues = {
                attachmentClearColour,
            },
            .depthClearValue = {},
            .stencilClearValue = {},
        };

        auto capture = commandBuffer.beginCapture();

        capture.bindDescriptorSets(renderer::DescriptorSetBindPoint::RENDER, pipelineLayout, 0, {descriptorSet});

        auto render = capture.beginRenderPass(renderPassBeginInfo);

        render.bindPipeline(pipeline);

        renderer::Viewport viewport = {
            .position = {
                .x = 0.0,
                .y = 0.0,
            },
            .extent = {
                .width = static_cast<float>(swapchain.getExtent().width),
                .height = static_cast<float>(swapchain.getExtent().height),
            },
            .depth = {
                .min = 0.0,
                .max = 1.0,
            },
        };

        renderer::Scissor scissor = {
            .offset = {0, 0},
            .extent = swapchain.getExtent(),
        };

        render.setPipelineViewports({viewport}, 0);
        render.setPipelineScissors({scissor}, 0);

        render.pushConstants(pipelineLayout, renderer::DescriptorShaderStageFlags::VERTEX, {reinterpret_cast<std::uint8_t*>(&pushConstant), sizeof(data::Position2D<float>)}, 0);

        render.bindVertexBuffers({vertexBuffer}, {0}, 0);

        render.draw(3, 1, 0, 0);

        render.end();
        capture.end();

        renderer::SubmitInfo submitInfo = {
            .fence = {fence},
            .commandBuffers = {commandBuffer},
            .waits = {acquireSemaphore},
            .signals = {submitSemaphore},
            .waitFlags = {renderer::SubmitWaitFlags::COLOR_ATTACHMENT_OUTPUT},
        };

        renderQueue.submit(submitInfo);

        swapchain.presentNextImage(submitSemaphore);

        frameIndex = (frameIndex + 1) % frameCount;
    }

    device.waitIdle();
}

int main() {
    try {
        run();

        return 0;
    }
    catch (const std::exception& exception) {
        std::println("Runtime exception: {}", exception.what());

        return 1;
    }
}